struct NormalizeParams {
	float* peaks;
	float max;
};
void normalize(float* data, int start, int size, NormalizeParams* params) {
	for (; size; size--, start++)
		data[start] = data[start] * params->max / params->peaks[start];

}