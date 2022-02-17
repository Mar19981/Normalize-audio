#pragma once

#define NOMINMAX

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <windows.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <vector>
#include <sstream>
#include "ui_Normalize.h"
#include "AudioFile.h"

struct NormalizeParams {
    float* peaks;
    float max;
};

typedef void(__cdecl* NORMALIZE)(float*, int, int, NormalizeParams*);

class Normalize : public QMainWindow
{
    Q_OBJECT

public:
    Normalize(QWidget *parent = Q_NULLPTR);

private:
    Ui::NormalizeClass ui;
    AudioFile<float> input, output;
    std::vector<float> processSamples, peaks;
    std::unique_ptr<NormalizeParams> params;
    void retrieveSamples();
    void retrieveOutput();
private slots:
    void normalize();
    void loadFile();
};
