#include "Normalize.h"

Normalize::Normalize(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    params = std::make_unique<NormalizeParams>();
    ui.threads->setValue(std::thread::hardware_concurrency());
    connect(ui.loadButton, &QPushButton::clicked, this, &Normalize::loadFile);
    connect(ui.normalizeButton, &QPushButton::clicked, this, &Normalize::normalize);  
    connect(ui.saveButton, &QPushButton::clicked, this, [=] {
        auto path = QFileDialog::getSaveFileName(this, "Zapisz jako", QDir::homePath(), "WAV (*.wav)");
        if (path.isNull() || path.isEmpty()) return;
        output.save(path.toStdString());
        });

}

void Normalize::retrieveSamples()
{
    int channels = input.getNumChannels(), samplesPerChannel = input.getNumSamplesPerChannel();
    params->max = std::abs(input.samples[0][0]);
    for (int i = 0; i < channels; i++) {
        for (int j = 0; j < samplesPerChannel; j++) {
            processSamples.emplace_back(input.samples[i][j]);
            params->max = std::max(params->max, std::abs(input.samples[i][j]));
            peaks.emplace_back(params->max);
        }
    }
    params->peaks = peaks.data();
}

void Normalize::retrieveOutput()
{
    int channels = input.getNumChannels(), samplesPerChannel = input.getNumSamplesPerChannel();
    for (int i = 0; i < channels; i++) {
        for (int j = 0; j < samplesPerChannel; j++) {
            output.samples[i][j] = processSamples.at(i * samplesPerChannel + j);
        }
    }
}

void Normalize::loadFile()
{
    auto path = QFileDialog::getOpenFileName(this, "Wybierz plik", QDir::homePath(), "WAV (*.wav)");
    if (path.isNull() || path.isEmpty()) return;
    input.load(path.toStdString());
    output = input;
    ui.normalizeButton->setEnabled(true);
}

void Normalize::normalize()
{
    processSamples.clear();
    retrieveSamples();

    int size = processSamples.size(), numThreads = ui.threads->value(), processSize = size / numThreads;
    std::vector<std::thread> threads(numThreads);

    HINSTANCE hDll{};
    NORMALIZE functionPtr{};
    hDll = LoadLibrary(ui.cppRadio->isChecked() ? TEXT("NormalizeC++.dll") : TEXT("NormalizeAsm.dll"));
    if (!hDll) throw std::runtime_error("Failed to load library");
    functionPtr = (NORMALIZE)GetProcAddress(hDll, "normalize");
    if (!functionPtr) throw std::runtime_error("Failed to load function");
    int start{};
    for (int i = 0; i < numThreads - 1; i++) {
        threads.at(i) = std::thread{ functionPtr, processSamples.data(), start, processSize, params.get() };
        start += processSize;
    }
    threads.at(numThreads - 1) = std::thread{ functionPtr, processSamples.data(), start, size - processSize * (numThreads - 1), params.get() };

    auto t1 = std::chrono::high_resolution_clock::now();
    for (auto& thread : threads)
        if (thread.joinable()) thread.join();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::stringstream ss;
    ss << "Czas trwania (s): " << input.getLengthInSeconds() << "\tBiblioteka: "
        << (ui.cppRadio->isChecked() ? "C++" : "Asembler") << "\tWatki: " << numThreads << "\tCzas: " 
        << std::chrono::duration<double, std::milli>(t2 - t1).count() << "ms\n\n";
    ui.results->setPlainText(ui.results->toPlainText() + ss.str().c_str());
    FreeLibrary(hDll);
    retrieveOutput();
    ui.saveButton->setEnabled(true);
}
