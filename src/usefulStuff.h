#ifndef USEFULSTUFF_H_
#define USEFULSTUFF_H_


#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD(x) ((x)*(M_PI/180.f))
#define RAD2DEG(x) ((x)*(180.f/M_PI))

#define errLog(message) \
	fprintf(stderr, "\nFile: %s, Function: %s, Line: %d, Note: %s\n", __FILE__, __FUNCTION__, __LINE__, message);



static inline int maxi(const int a, const int b){
    return (a > b) ? a : b;
}

static inline int mini(const int a, const int b)
{
    return (a < b) ? a : b;
}

static inline float maxf(const float a, const float b)
{
    return (a > b) ? a : b;
}

static inline float minf(const float a, const float b)
{
    return (a < b) ? a : b;
}

static inline float clampf(const float value, const float min, const float max) 
{
    const float t = value < min ? min : value;
    return t > max ? max : t;
}

static inline float cosLerp(const float y1, const float y2, const float mu)
{
	const double mu2 = (1-cos(mu*M_PI))/2;
	return(y1*(1-mu2)+y2*mu2);
}

static inline float lerp(const float s, const float e, const float t)
{
	return s + (e - s) * t;
}


static inline float blerp(const float c00, const float c10, const float c01, const float c11, const float tx, const float ty)
{
	//    return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
	const float s = c00 + (c10 - c00) * tx;
	const float e = c01 + (c11 - c01) * tx;
	return (s + (e - s) * ty);
}

#endif /*USEFULSTUFF_H_*/