// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "PixelBufferUtilsGamma8bit.h"

#include <scene_rdl2/common/math/MathUtil.h>
#include <scene_rdl2/common/math/Color.h>

#include <tbb/parallel_for.h>

#include <math.h>
#include <limits.h>

// Define this to use a table for the gamma correction. In practice this is much
// faster than calling powf repeatedly.
#define USE_TABLE_FOR_GAMMA

#define GAMMA_EXPONENT          (1.0f / 2.2f)

// If an image doesn't have at least this much difference
// between min and max values, do not apply a scale when normalizing
#define MIN_NORMALIZED_DISTANCE (0.001f)

namespace scene_rdl2 {
namespace fb_util {

using namespace scene_rdl2::math;

namespace {
    
// Mimics "for (unsigned i = start; i != end; ++i)" behavior.
// Calling this version allows the caller to decide at runtime whether they
// want it parallelized or not.
template <typename T, typename FUNC>
finline void
simpleLoop(bool parallel, T start, T end, const FUNC &func)
{
    if (parallel) {
        tbb::parallel_for (start, end, [&](unsigned iter) {
            func(iter);
        });
    } else {
        for (unsigned iter = start; iter != end; ++iter) {
            func(iter);
        }
    }
}

#ifdef USE_TABLE_FOR_GAMMA

// Gamma tables and conversion scheme courtesy of Mark Davis.
static const float sGammaTable1[] =
{
    255.000000f, 255.113205f, 255.226135f, 255.339081f, 255.452011f, 255.564941f, 255.677643f, 255.790573f,
    255.903259f, 256.015686f, 256.128387f, 256.240814f, 256.353241f, 256.465668f, 256.578094f, 256.690277f,
    256.802460f, 256.914642f, 257.026825f, 257.138702f, 257.250885f, 257.362823f, 257.474487f, 257.586426f,
    257.698059f, 257.809723f, 257.921387f, 258.033081f, 258.144470f, 258.255890f, 258.367310f, 258.478699f,
    258.590118f, 258.701263f, 258.812439f, 258.923584f, 259.034485f, 259.145630f, 259.256531f, 259.367462f,
    259.478363f, 259.588989f, 259.699646f, 259.810303f, 259.920929f, 260.031586f, 260.141968f, 260.252350f,
    260.362762f, 260.473145f, 260.583527f, 260.693665f, 260.803802f, 260.913940f, 261.023804f, 261.133942f,
    261.243835f, 261.353699f, 261.463593f, 261.573212f, 261.682831f, 261.792450f, 261.902069f, 262.011719f,
    262.121094f, 262.230713f, 262.340088f, 262.449432f, 262.558563f, 262.667908f, 262.777069f, 262.886169f,
    262.995026f, 263.104126f, 263.212982f, 263.321869f, 263.430725f, 263.539581f, 263.648193f, 263.757080f,
    263.865662f, 263.974274f, 264.082611f, 264.191223f, 264.299591f, 264.407928f, 264.516266f, 264.624390f,
    264.732727f, 264.840820f, 264.948914f, 265.056763f, 265.164825f, 265.272705f, 265.380524f, 265.488373f,
    265.596222f, 265.704041f, 265.811646f, 265.919220f, 266.026794f, 266.134399f, 266.241730f, 266.349091f,
    266.456390f, 266.563721f, 266.671051f, 266.778137f, 266.885468f, 266.992523f, 267.099640f, 267.206421f,
    267.313538f, 267.420319f, 267.527161f, 267.633972f, 267.740814f, 267.847351f, 267.953949f, 268.060486f,
    268.167084f, 268.273621f, 268.379944f, 268.486511f, 268.592834f, 268.699127f, 268.805176f, 268.911499f,
    269.017548f, 269.123627f, 269.229645f, 269.335724f, 269.441528f, 269.547333f, 269.653137f, 269.758911f,
    269.864716f, 269.970551f, 270.076080f, 270.181610f, 270.287170f, 270.392700f, 270.498016f, 270.603302f,
    270.708832f, 270.814148f, 270.919189f, 271.024475f, 271.129486f, 271.234802f, 271.339844f, 271.444641f,
    271.549652f, 271.654419f, 271.759491f, 271.864258f, 271.969025f, 272.073547f, 272.178345f, 272.282867f,
    272.387421f, 272.491913f, 272.596436f, 272.700714f, 272.804993f, 272.909546f, 273.013794f, 273.117798f,
    273.222076f, 273.326111f, 273.430389f, 273.534393f, 273.638153f, 273.742188f, 273.846161f, 273.949951f,
    274.053711f, 274.157471f, 274.261230f, 274.364746f, 274.468506f, 274.572021f, 274.675507f, 274.779022f,
    274.882294f, 274.985779f, 275.089050f, 275.192291f, 275.295563f, 275.398804f, 275.501801f, 275.604797f,
    275.708038f, 275.811035f, 275.913788f, 276.016785f, 276.119781f, 276.222534f, 276.325256f, 276.428009f,
    276.530731f, 276.633240f, 276.735718f, 276.838440f, 276.940948f, 277.043427f, 277.145660f, 277.248138f,
    277.350403f, 277.452637f, 277.554840f, 277.657074f, 277.759064f, 277.861298f, 277.963257f, 278.065216f,
    278.167236f, 278.269196f, 278.370911f, 278.472900f, 278.574615f, 278.676331f, 278.778076f, 278.879547f,
    278.981262f, 279.082703f, 279.184174f, 279.285675f, 279.387115f, 279.488586f, 279.589783f, 279.691040f,
    279.792236f, 279.893433f, 279.994629f, 280.095886f, 280.196838f, 280.297760f, 280.398743f, 280.499695f,
    280.600647f, 280.701599f, 280.802338f, 280.903015f, 281.003723f, 281.104431f, 281.205139f, 281.305573f,
    281.406281f, 281.506714f, 281.607147f, 281.707611f, 281.808075f, 281.908264f, 282.008453f, 282.108887f,
    282.209106f, 282.309296f, 282.409241f, 282.509430f, 282.609344f, 282.709290f, 282.809235f, 282.909180f,
    283.009094f, 283.108795f, 283.208740f, 283.308411f, 283.408081f, 283.507751f, 283.607208f, 283.706879f,
    283.806305f, 283.905975f, 284.005432f, 284.104858f, 284.204041f, 284.303436f, 284.402618f, 284.501801f,
    284.601227f, 284.700134f, 284.799316f, 284.898468f, 284.997406f, 285.096344f, 285.195251f, 285.294159f,
    285.393066f, 285.492004f, 285.590637f, 285.689575f, 285.788269f, 285.886902f, 285.985321f, 286.083954f,
    286.182373f, 286.281067f, 286.379456f, 286.477875f, 286.576294f, 286.674408f, 286.772827f, 286.871002f,
    286.969391f, 287.067566f, 287.165710f, 287.263580f, 287.361755f, 287.459656f, 287.557800f, 287.655701f,
    287.753601f, 287.851227f, 287.949127f, 288.046783f, 288.144684f, 288.242310f, 288.339966f, 288.437622f,
    288.535278f, 288.632629f, 288.730286f, 288.827698f, 288.925079f, 289.022430f, 289.119843f, 289.216980f,
    289.314362f, 289.411499f, 289.508606f, 289.605774f, 289.702911f, 289.800049f, 289.896912f, 289.993774f,
    290.090912f, 290.187805f, 290.284668f, 290.381561f, 290.478180f, 290.575073f, 290.671661f, 290.768311f,
    290.864929f, 290.961548f, 291.058167f, 291.154541f, 291.251190f, 291.347565f, 291.443909f, 291.540253f,
    291.636627f, 291.733002f, 291.829132f, 291.925476f, 292.021606f, 292.117706f, 292.213837f, 292.309937f,
    292.405823f, 292.501923f, 292.597778f, 292.693634f, 292.789490f, 292.885376f, 292.981232f, 293.076813f,
    293.172699f, 293.268280f, 293.363892f, 293.459503f, 293.555084f, 293.650696f, 293.746063f, 293.841644f,
    293.937012f, 294.032349f, 294.127686f, 294.223053f, 294.318390f, 294.413483f, 294.508820f, 294.603912f,
    294.699036f, 294.794128f, 294.889221f, 294.984314f, 295.079163f, 295.173981f, 295.269073f, 295.363922f,
    295.458740f, 295.553589f, 295.648193f, 295.743011f, 295.837616f, 295.932190f, 296.027008f, 296.121613f,
    296.215912f, 296.310547f, 296.405121f, 296.499420f, 296.593750f, 296.688080f, 296.782440f, 296.876770f,
    296.971069f, 297.065155f, 297.159485f, 297.253571f, 297.347626f, 297.441711f, 297.535767f, 297.629852f,
    297.723694f, 297.817749f, 297.911560f, 298.005371f, 298.099213f, 298.193024f, 298.286835f, 298.380432f,
    298.474243f, 298.567780f, 298.661346f, 298.754944f, 298.848480f, 298.942047f, 299.035614f, 299.128937f,
    299.222229f, 299.315796f, 299.409119f, 299.502411f, 299.595734f, 299.688782f, 299.782074f, 299.875153f,
    299.968201f, 300.061493f, 300.154327f, 300.247345f, 300.340393f, 300.433472f, 300.526276f, 300.619080f,
    300.712128f, 300.804901f, 300.897461f, 300.990265f, 301.083069f, 301.175598f, 301.268402f, 301.360962f,
    301.453491f, 301.546051f, 301.638580f, 301.731140f, 301.823425f, 301.915955f, 302.008270f, 302.100525f,
    302.192841f, 302.285126f, 302.377411f, 302.469452f, 302.561737f, 302.653748f, 302.746063f, 302.838074f,
    302.930145f, 303.021912f, 303.113953f, 303.205963f, 303.297760f, 303.389526f, 303.481567f, 303.573334f,
    303.665131f, 303.756653f, 303.848450f, 303.940216f, 304.031738f, 304.123260f, 304.214783f, 304.306580f,
    304.397827f, 304.489349f, 304.580872f, 304.672150f, 304.763702f, 304.854950f, 304.946228f, 305.037476f,
    305.128754f, 305.220032f, 305.311035f, 305.402313f, 305.493347f, 305.584351f, 305.675354f, 305.766388f,
    305.857391f, 305.948395f, 306.039154f, 306.130188f, 306.220947f, 306.311676f, 306.402466f, 306.493225f,
    306.583954f, 306.674744f, 306.765259f, 306.856018f, 306.946503f, 307.037018f, 307.127533f, 307.218018f,
    307.308533f, 307.399048f, 307.489288f, 307.579773f, 307.670044f, 307.760284f, 307.850525f, 307.940765f,
    308.031036f, 308.121307f, 308.211273f, 308.301514f, 308.391541f, 308.481537f, 308.571533f, 308.661499f,
    308.751495f, 308.841492f, 308.931244f, 309.021210f, 309.110962f, 309.200714f, 309.290710f, 309.380463f,
    309.469940f, 309.559662f, 309.649414f, 309.738892f, 309.828644f, 309.918121f, 310.007629f, 310.097076f,
    310.186584f, 310.275818f, 310.365295f, 310.454529f, 310.544037f, 310.633270f, 310.722473f, 310.811707f,
    310.900940f, 310.990173f, 311.079407f, 311.168365f, 311.257599f, 311.346588f, 311.435547f, 311.524536f,
    311.613495f, 311.702454f, 311.791443f, 311.880157f, 311.969147f, 312.057861f, 312.146576f, 312.235321f,
    312.324036f, 312.412750f, 312.501465f, 312.589935f, 312.678650f, 312.767120f, 312.855835f, 312.944305f,
    313.032776f, 313.121246f, 313.209717f, 313.297913f, 313.386383f, 313.474609f, 313.563049f, 313.651276f,
    313.739471f, 313.827698f, 313.915894f, 314.003845f, 314.092072f, 314.180267f, 314.268219f, 314.356171f,
    314.444397f, 314.532349f, 314.620300f, 314.708008f, 314.795959f, 314.883911f, 314.971588f, 315.059296f,
    315.147278f, 315.234985f, 315.322693f, 315.410370f, 315.498077f, 315.585510f, 315.673218f, 315.760651f,
    315.848358f, 315.935791f, 316.023254f, 316.110718f, 316.198151f, 316.285339f, 316.372772f, 316.460236f,
    316.547394f, 316.634613f, 316.721802f, 316.809265f, 316.896423f, 316.983368f, 317.070557f, 317.157776f,
    317.244690f, 317.331879f, 317.418823f, 317.505737f, 317.592682f, 317.679626f, 317.766571f, 317.853485f,
    317.940155f, 318.027100f, 318.113800f, 318.200745f, 318.287415f, 318.374084f, 318.460785f, 318.547455f,
    318.633881f, 318.720551f, 318.807007f, 318.893677f, 318.980103f, 319.066498f, 319.152954f, 319.239380f,
    319.325806f, 319.412201f, 319.498657f, 319.584808f, 319.670990f, 319.757385f, 319.843597f, 319.929749f,
    320.015900f, 320.102112f, 320.188263f, 320.274170f, 320.360352f, 320.446259f, 320.532166f, 320.618378f,
    320.704285f, 320.790192f, 320.876129f, 320.961761f, 321.047668f, 321.133606f, 321.219269f, 321.304901f,
    321.390839f, 321.476501f, 321.562134f, 321.647827f, 321.733215f, 321.818878f, 321.904541f, 321.989929f,
    322.075623f, 322.161011f, 322.246399f, 322.331848f, 322.417236f, 322.502655f, 322.588043f, 322.673187f,
    322.758606f, 322.843750f, 322.929169f, 323.014313f, 323.099487f, 323.184601f, 323.269745f, 323.354675f,
    323.439789f, 323.524963f, 323.609863f, 323.694763f, 323.779907f, 323.864807f, 323.949707f, 324.034607f,
    324.119507f, 324.204407f, 324.289032f, 324.373932f, 324.458557f, 324.543213f, 324.628113f, 324.712738f,
    324.797394f, 324.882050f, 324.966400f, 325.051056f, 325.135681f, 325.220093f, 325.304718f, 325.389130f,
    325.473480f, 325.557892f, 325.642273f, 325.726654f, 325.811035f, 325.895172f, 325.979553f, 326.063690f,
    326.148102f, 326.232208f, 326.316345f, 326.400452f, 326.484619f, 326.568726f, 326.652863f, 326.736755f,
    326.820862f, 326.904755f, 326.988861f, 327.072754f, 327.156647f, 327.240509f, 327.324402f, 327.408264f,
    327.491882f, 327.575775f, 327.659637f, 327.743256f, 327.826904f, 327.910736f, 327.994385f, 328.077972f,
    328.161621f, 328.244995f, 328.328583f, 328.412231f, 328.495605f, 328.579193f, 328.662598f, 328.745972f,
    328.829559f, 328.912933f, 328.996063f, 329.079407f, 329.162781f, 329.246155f, 329.329285f, 329.412628f,
    329.495728f, 329.578857f, 329.661957f, 329.745087f, 329.828186f, 329.911316f, 329.994415f, 330.077271f,
    330.160400f, 330.243225f, 330.326355f, 330.409210f, 330.492065f, 330.574890f, 330.657776f, 330.740631f,
    330.823486f, 330.906067f, 330.988953f, 331.071533f, 331.154419f, 331.237000f, 331.319611f, 331.402191f,
    331.484802f, 331.567413f, 331.649994f, 331.732605f, 331.814972f, 331.897583f, 331.979919f, 332.062500f,
    332.144836f, 332.227203f, 332.309570f, 332.391907f, 332.474243f, 332.556335f, 332.638672f, 332.720764f,
    332.803131f, 332.885223f, 332.967285f, 333.049652f, 333.131744f, 333.213837f, 333.295929f, 333.377777f,
    333.459869f, 333.541931f, 333.623779f, 333.705627f, 333.787720f, 333.869537f, 333.951385f, 334.033234f,
    334.115082f, 334.196899f, 334.278717f, 334.360321f, 334.442139f, 334.523743f, 334.605560f, 334.687164f,
    334.768738f, 334.850311f, 334.931915f, 335.013489f, 335.095062f, 335.176361f, 335.257965f, 335.339539f,
    335.420868f, 335.502197f, 335.583771f, 335.665100f, 335.746429f, 335.827759f, 335.909088f, 335.990173f,
    336.071503f, 336.152832f, 336.233887f, 336.314972f, 336.396301f, 336.477386f, 336.558441f, 336.639526f,
    336.720581f, 336.801666f, 336.882721f, 336.963562f, 337.044617f, 337.125427f, 337.206512f, 337.287323f,
    337.368134f, 337.449219f, 337.530029f, 337.610596f, 337.691406f, 337.772217f, 337.853027f, 337.933594f,
    338.014404f, 338.094971f, 338.175537f, 338.256348f, 338.336914f, 338.417480f, 338.498047f, 338.578583f,
    338.658936f, 338.739502f, 338.820038f, 338.900360f, 338.980927f, 339.061218f, 339.141510f, 339.221832f,
    339.302124f, 339.382446f, 339.462738f, 339.543060f, 339.623383f, 339.703430f, 339.783722f, 339.863770f,
    339.943817f, 340.024139f, 340.104187f, 340.184235f, 340.264313f, 340.344360f, 340.424164f, 340.504211f,
    340.584259f, 340.664032f, 340.744080f, 340.823914f, 340.903717f, 340.983765f, 341.063538f, 341.143341f,
    341.223114f, 341.302704f, 341.382477f, 341.462280f, 341.541809f, 341.621613f, 341.701141f, 341.780701f,
    341.860504f, 341.940033f, 342.019562f, 342.099121f, 342.178680f, 342.258209f, 342.337494f, 342.417023f,
    342.496307f, 342.575867f, 342.655151f, 342.734436f, 342.813721f, 342.893280f, 342.972565f, 343.051849f,
    343.130859f, 343.210144f, 343.289459f, 343.368500f, 343.447754f, 343.526794f, 343.605835f, 343.685120f,
    343.764160f, 343.843170f, 343.922241f, 344.001251f, 344.080292f, 344.159088f, 344.238098f, 344.316864f,
    344.395905f, 344.474701f, 344.553467f, 344.632477f, 344.711273f, 344.790039f, 344.868835f, 344.947632f,
    345.026123f, 345.104889f, 345.183655f, 345.262207f, 345.340973f, 345.419495f, 345.498047f, 345.576813f,
    345.655304f, 345.733856f, 345.812378f, 345.890625f, 345.969177f, 346.047668f, 346.126190f, 346.204468f,
    346.282990f, 346.361267f, 346.439545f, 346.517792f, 346.596069f, 346.674347f, 346.752594f, 346.830872f,
    346.909149f, 346.987396f, 347.065430f, 347.143677f, 347.221710f, 347.299713f, 347.377960f, 347.455994f,
    347.533997f, 347.612030f, 347.690033f, 347.768036f, 347.846069f, 347.923798f, 348.001831f, 348.079834f,
    348.157623f, 348.235352f, 348.313354f, 348.391144f, 348.468872f, 348.546661f, 348.624390f, 348.702179f,
    348.779907f, 348.857422f, 348.935181f, 349.012665f, 349.090424f, 349.167938f, 349.245697f, 349.323181f,
};

MNRY_STATIC_ASSERT(sizeof(sGammaTable1) == 4096);

static const float sGammaTable2[] =
{
    0,5.74369e-18,7.87087e-18,1.07859e-17,1.47804e-17,2.02544e-17,2.77556e-17,3.80349e-17,
    5.21211e-17,7.14242e-17,9.78763e-17,1.34125e-16,1.83798e-16,2.51868e-16,3.45148e-16,4.72973e-16,
    6.48139e-16,8.88178e-16,1.21712e-15,1.66788e-15,2.28558e-15,3.13204e-15,4.292e-15,5.88154e-15,
    8.05978e-15,1.10447e-14,1.51351e-14,2.07405e-14,2.84217e-14,3.89477e-14,5.3372e-14,7.31384e-14,
    1.00225e-13,1.37344e-13,1.88209e-13,2.57913e-13,3.53431e-13,4.84325e-13,6.63695e-13,9.09495e-13,
    1.24633e-12,1.70791e-12,2.34043e-12,3.20721e-12,4.395e-12,6.0227e-12,8.25321e-12,1.13098e-11,
    1.54984e-11,2.12382e-11,2.91038e-11,3.98825e-11,5.4653e-11,7.48938e-11,1.02631e-10,1.4064e-10,
    1.92726e-10,2.64103e-10,3.61913e-10,4.95948e-10,6.79623e-10,9.31323e-10,1.27624e-09,1.7489e-09,
    2.3966e-09,3.28418e-09,4.50048e-09,6.16724e-09,8.45129e-09,1.15812e-08,1.58703e-08,2.17479e-08,
    2.98023e-08,4.08396e-08,5.59646e-08,7.66912e-08,1.05094e-07,1.44015e-07,1.97352e-07,2.70441e-07,
    3.70599e-07,5.07851e-07,6.95934e-07,9.53674e-07,1.30687e-06,1.79087e-06,2.45412e-06,3.363e-06,
    4.6085e-06,6.31526e-06,8.65412e-06,1.18592e-05,1.62512e-05,2.22699e-05,3.05176e-05,4.18198e-05,
    5.73078e-05,7.85318e-05,0.000107616,0.000147472,0.000202088,0.000276932,0.000379494,0.00052004,
    0.000712637,0.000976562,0.00133823,0.00183385,0.00251302,0.00344372,0.0047191,0.00646682,
    0.00886182,0.0121438,0.0166413,0.0228044,0.03125,0.0428235,0.0586832,0.0804166,
    0.110199,0.151011,0.206938,0.283578,0.388602,0.532521,0.72974,1,
    1.37035,1.87786,2.57333,3.52637,4.83236,6.62203,9.0745,12.4353,
    17.0407,23.3517,32,43.8512,60.0916,82.3465,112.844,154.635,
    211.905,290.384,397.928,545.301,747.254,1024,1403.24,1922.93,
    2635.09,3611,4948.33,6780.96,9292.29,12733.7,17449.6,23912.1,
    32768,44903.7,61533.8,84322.9,115552,158347,216991,297353,
    407478,558388,  765188,1048576,1.43692e+06,1.96908e+06,2.69833e+06,3.69766e+06,
    5.06709e+06,6943698,9515303,13039305,17868424,24486012,33554432,45981348,
    63010588,86346616,118325176,162147024,222198336,304489696,417257760,571789568,
    783552384,1073741824,1471403136,2016338816,2763091712,3786405632,5188704768,7110346752,
    9743670272,13352248320,18297266176,25073676288,34359738368,47084900352,64522842112,88418934784,
    121164980224,166038552576,227531096064,311797448704,427271946240,585512517632,802357641216,1099511627776,
    1506716811264,2064730947584,2829405913088,3877279367168,5313233682432,7280995074048,9977518358528,13672702279680,
    18736400564224,25675444518912,35184372088832,48214937960448,66071390322688,90540989218816,124072939749376,170023477837824,
    232991842369536,319280587472896,437526472949760,599564818055168,821614224605184,1125899906842624,1542878014734336,2114284490326016,
    2897311655002112,3970334071980032,5440751290810368,7455738955825152,10216978799132672,14000847134392320,19186074177765376,26291655187365888,
    36028797018963968,49372096471498752,67657103690432512,92713972960067584,127050690303361024,174104041305931776,238583646586404864,0
};

MNRY_STATIC_ASSERT(sizeof(sGammaTable2) == 1024);

#endif  // #ifdef USE_TABLE_FOR_GAMMA

// http://en.wikipedia.org/wiki/Ordered_dithering
static const float sDitherMatrix[8][8] =
{
     1.f/65.f,   49.f/65.f,   13.f/65.f,   61.f/65.f,    4.f/65.f,   52.f/65.f,   16.f/65.f,   64.f/65.f,
    33.f/65.f,   17.f/65.f,   45.f/65.f,   29.f/65.f,   36.f/65.f,   20.f/65.f,   48.f/65.f,   32.f/65.f,
     9.f/65.f,   57.f/65.f,    5.f/65.f,   53.f/65.f,   12.f/65.f,   60.f/65.f,    8.f/65.f,   56.f/65.f,
    41.f/65.f,   25.f/65.f,   37.f/65.f,   21.f/65.f,   44.f/65.f,   28.f/65.f,   40.f/65.f,   24.f/65.f,
     3.f/65.f,   51.f/65.f,   15.f/65.f,   63.f/65.f,    2.f/65.f,   50.f/65.f,   14.f/65.f,   62.f/65.f,
    35.f/65.f,   19.f/65.f,   47.f/65.f,   31.f/65.f,   34.f/65.f,   18.f/65.f,   46.f/65.f,   30.f/65.f,
    11.f/65.f,   59.f/65.f,    7.f/65.f,   55.f/65.f,   10.f/65.f,   58.f/65.f,    6.f/65.f,   54.f/65.f,
    43.f/65.f,   27.f/65.f,   39.f/65.f,   23.f/65.f,   42.f/65.f,   26.f/65.f,   38.f/65.f,   22.f/65.f,
};


// Transform an existing PixelBuffer into a different buffer type using a
// custom functor.
template<typename DEST_PIXEL_TYPE, typename SRC_PIXEL_TYPE, typename BODY>
finline void
processPixelBuffer(PixelBuffer<DEST_PIXEL_TYPE>& destBuffer, const PixelBuffer<SRC_PIXEL_TYPE>& srcBuffer,
                    BODY const &body, bool parallel)
{
    unsigned w = srcBuffer.getWidth();
    unsigned h = srcBuffer.getHeight();

    destBuffer.init(w, h);

    simpleLoop(parallel, 0u, h, [&](unsigned y) {

        DEST_PIXEL_TYPE *dst = destBuffer.getRow(y);
        const SRC_PIXEL_TYPE *src = srcBuffer.getRow(y);

        for (unsigned x = 0; x < w; ++x) {
            body(dst, *src, x, y);
            ++dst;
            ++src;
        }
    });
}

// Input [0, 1] -> output [0, 255]
finline void
gammaCorrectColorComponent(float &x)
{
#ifdef USE_TABLE_FOR_GAMMA

    union VARIANT {
        float f;
        unsigned int u;
    };
    VARIANT *v = (VARIANT *) &x;
    x = sGammaTable1[(v->u >> 13) & 0x3ff] * sGammaTable2[(v->u >> 23) & 0xff];

#else

    // Crazy slow!!
    x = powf(x, GAMMA_EXPONENT) * 255.f;

#endif
}

finline uint8_t
gammaCorrectDitherQuantize(float in, unsigned int x, unsigned int y)
{
    gammaCorrectColorComponent(in);
    return uint8_t(in + sDitherMatrix[y & 7][x & 7]);
}

template <typename SRC_BUFFER_TYPE>
void
computeNormalizedScaleAndOffset(Vec3f &offset, float &scale, const SRC_BUFFER_TYPE &srcBuffer)
{
    offset = Vec3f(0.f);
    scale = 1.f;

    Vec3f min = Vec3f(std::numeric_limits<float>::max());
    Vec3f max = Vec3f(std::numeric_limits<float>::min());

    const unsigned int w = srcBuffer.getWidth();
    const unsigned int h = srcBuffer.getHeight();
    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
            const auto &p = srcBuffer.getPixel(x, y);
            if (math::isfinite(p[0])) {
                if (p[0] < min[0]) min[0] = p[0];
                if (p[0] > max[0]) max[0] = p[0];
            }
            if (math::isfinite(p[1])) {
                if (p[1] < min[1]) min[1] = p[1];
                if (p[1] > max[1]) max[1] = p[1];
            }
            if (math::isfinite(p[2])) {
                if (p[2] < min[2]) min[2] = p[2];
                if (p[2] > max[2]) max[2] = p[2];
            }
        }
    }
    const Vec3f diff = max - min;
    const float maxDiff = std::max(diff[0], std::max(diff[1], diff[2]));
    if (maxDiff > MIN_NORMALIZED_DISTANCE) {
        scale = 1.f / maxDiff;
    }
    offset = -min;
}

void
computeNormalizedScaleAndOffset(Vec2f &offset, float &scale, const Float2Buffer &srcBuffer)
{
    offset = Vec2f(0.f);
    scale = 1.f;

    Vec2f min = Vec2f(std::numeric_limits<float>::max());
    Vec2f max = Vec2f(std::numeric_limits<float>::min());

    const unsigned int w = srcBuffer.getWidth();
    const unsigned int h = srcBuffer.getHeight();
    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
            const auto &p = srcBuffer.getPixel(x, y);
            if (math::isfinite(p[0])) {
                if (p[0] < min[0]) min[0] = p[0];
                if (p[0] > max[0]) max[0] = p[0];
            }
            if (math::isfinite(p[1])) {
                if (p[1] < min[1]) min[1] = p[1];
                if (p[1] > max[1]) max[1] = p[1];
            }
        }
    }
    const Vec2f diff = max - min;
    const float maxDiff = std::max(diff[0], diff[1]);
    if (maxDiff > MIN_NORMALIZED_DISTANCE) {
        scale = 1.f / maxDiff;
    }
    offset = -min;
}


void
computeNormalizedScaleAndOffset(float &offset, float &scale, const FloatBuffer &srcBuffer)
{
    offset = 0.f;
    scale = 1.f;

    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::min();

    const unsigned int w = srcBuffer.getWidth();
    const unsigned int h = srcBuffer.getHeight();
    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
            const float p = srcBuffer.getPixel(x, y);
            if (math::isfinite(p) && p > max) max = p;
            if (math::isfinite(p) && p < min) min = p;
        }
    }
    const float diff = max - min;
    if (diff > MIN_NORMALIZED_DISTANCE) {
        scale = 1.f / diff;
    }
    offset = -min;
}

}   // End of anon namespace.


void
gammaAndQuantizeTo8bit(Rgb888Buffer& destBuffer, 
                       const RenderBuffer& srcBuffer,
                       PixelBufferUtilOptions options, 
                       float exposure, float gamma)
{
    const bool applyGamma = options & PIXEL_BUFFER_UTIL_OPTIONS_APPLY_GAMMA;
    const bool normalize  = options & PIXEL_BUFFER_UTIL_OPTIONS_NORMALIZE;
    const bool parallel   = options & PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL;

    const RenderColor zeroColor(0.0f);
    const RenderColor oneColor(1.0f);

    // TODO: parallelize
    Vec3f offset(0.f);
    float scale(1.f);
    if (normalize) {
        computeNormalizedScaleAndOffset(offset, scale, srcBuffer);
    }

    processPixelBuffer(destBuffer, srcBuffer, [&](ByteColor *dest,
                                                  const RenderColor &src,
                                                  unsigned x, unsigned y) {
        RenderColor p = src;

        if (normalize) {
            p[0] = (p[0] + offset[0]) * scale;
            p[1] = (p[1] + offset[1]) * scale;
            p[2] = (p[2] + offset[2]) * scale;
        } else {          
            // Apply exposure
            float exposureScale = pow(2.f, exposure);
            p[0] *= exposureScale;
            p[1] *= exposureScale;
            p[2] *= exposureScale;

            // Apply user gamma
            p[0] = pow(p[0], 1.f / gamma);
            p[1] = pow(p[1], 1.f / gamma);
            p[2] = pow(p[2], 1.f / gamma);

            // Clamp pixels to 0.0 -> 1.0 range.
            p = min(p, oneColor);
            p = max(p, zeroColor);
        }

        if (applyGamma) {
            gammaCorrectColorComponent(p[0]);
            gammaCorrectColorComponent(p[1]);
            gammaCorrectColorComponent(p[2]);
        }

        // Dither and quantize to 8-bit.
        float ditherVal = sDitherMatrix[y & 7][x & 7];
        dest->r = uint8_t(p[0] + ditherVal);
        dest->g = uint8_t(p[1] + ditherVal);
        dest->b = uint8_t(p[2] + ditherVal);

    }, parallel);
}

static void
gammaAndQuantizeTo8bit(Rgb888Buffer& destBuffer, 
                       const FloatBuffer& srcBuffer,
                       PixelBufferUtilOptions options, 
                       float exposure, float gamma)
{
    const bool applyGamma = options & PIXEL_BUFFER_UTIL_OPTIONS_APPLY_GAMMA;
    const bool normalize  = options & PIXEL_BUFFER_UTIL_OPTIONS_NORMALIZE;
    const bool parallel   = options & PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL;

    float offset(0.f);
    float scale(1.f);
    if (normalize) {
        computeNormalizedScaleAndOffset(offset, scale, srcBuffer);
    }

    processPixelBuffer(destBuffer, srcBuffer, [&](ByteColor *dest,
                                                   const float &src,
                                                   unsigned x, unsigned y) {
        float p = src;

        if (normalize) {
            p = (p + offset) * scale;
        } else {
            // Apply exposure
            float exposureScale = pow(2.f, exposure);
            p *= exposureScale;

            // Apply user gamma
            p = pow(p, 1.f / gamma);

            // Clamp pixels to 0.0 -> 1.0 range.
            p = min(p, 1.f);
            p = max(p, 0.f);
        }

        if (applyGamma) {
            gammaCorrectColorComponent(p);
        }

        // Dither and quantize to 8-bit.
        float ditherVal = sDitherMatrix[y & 7][x & 7];
        dest->r = uint8_t(p + ditherVal);
        dest->g = dest->r;
        dest->b = dest->r;

    }, parallel);
}

static void
gammaAndQuantizeTo8bit(Rgb888Buffer& destBuffer, 
                       const Float2Buffer& srcBuffer,
                       PixelBufferUtilOptions options, 
                       float exposure, float gamma)
{
    const bool applyGamma = options & PIXEL_BUFFER_UTIL_OPTIONS_APPLY_GAMMA;
    const bool normalize  = options & PIXEL_BUFFER_UTIL_OPTIONS_NORMALIZE;
    const bool parallel   = options & PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL;

    // dest->r = src[0]
    // dest->g = src[1]
    // dest->b = 0

    const Vec2f zeroVec2f(0.f);
    const Vec2f oneVec2f(1.f);


    // TODO: parallelize
    Vec2f offset(0.f);
    float scale(1.f);
    if (normalize) {
        computeNormalizedScaleAndOffset(offset, scale, srcBuffer);
    }

    processPixelBuffer(destBuffer, srcBuffer, [&](ByteColor *dest,
                                                  const Vec2f &src,
                                                  unsigned x, unsigned y) {
        Vec2f p = src;


        if (normalize) {
            p = (p + offset) * scale;
        } else {
            // Apply exposure
            float exposureScale = pow(2.f, exposure);
            p[0] *= exposureScale;
            p[1] *= exposureScale;

            // Apply user gamma
            p[0] = pow(p[0], 1.f / gamma);
            p[1] = pow(p[1], 1.f / gamma);

            // Clamp pixels to 0.0 -> 1.0 range.
            p = min(p, oneVec2f);
            p = max(p, zeroVec2f);
        }

        if (applyGamma) {
            gammaCorrectColorComponent(p[0]);
            gammaCorrectColorComponent(p[1]);
        }

        // Dither and quantize to 8-bit.
        float ditherVal = sDitherMatrix[y & 7][x & 7];
        dest->r = uint8_t(p[0] + ditherVal);
        dest->g = uint8_t(p[1] + ditherVal);
        dest->b = 0.f;

    }, parallel);
}

static void
gammaAndQuantizeTo8bit(Rgb888Buffer& destBuffer, 
                       const Float3Buffer& srcBuffer,
                       PixelBufferUtilOptions options, 
                       float exposure, float gamma)
{
    const bool applyGamma = options & PIXEL_BUFFER_UTIL_OPTIONS_APPLY_GAMMA;
    const bool normalize  = options & PIXEL_BUFFER_UTIL_OPTIONS_NORMALIZE;
    const bool parallel   = options & PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL;

    const Vec3f zeroVec3f(0.f);
    const Vec3f oneVec3f(1.f);

    Vec3f offset(0.f);
    float scale(1.f);
    if (normalize) {
        computeNormalizedScaleAndOffset(offset, scale, srcBuffer);
    }

    processPixelBuffer(destBuffer, srcBuffer, [&](ByteColor *dest,
                                                  const Vec3f &src,
                                                  unsigned x, unsigned y) {
        Vec3f p = src;

        if (normalize) {
            p = (p + offset) * scale;
        } else {
            // Apply exposure
            float exposureScale = pow(2.f, exposure);
            p[0] *= exposureScale;
            p[1] *= exposureScale;
            p[2] *= exposureScale;

            // Apply user gamma
            p[0] = pow(p[0], 1.f / gamma);
            p[1] = pow(p[1], 1.f / gamma);
            p[2] = pow(p[2], 1.f / gamma);

            // Clamp pixels to 0.0 -> 1.0 range.
            p = min(p, oneVec3f);
            p = max(p, zeroVec3f);
        }

        if (applyGamma) {
            gammaCorrectColorComponent(p[0]);
            gammaCorrectColorComponent(p[1]);
            gammaCorrectColorComponent(p[2]);
        }

        // Dither and quantize to 8-bit.
        float ditherVal = sDitherMatrix[y & 7][x & 7];
        dest->r = uint8_t(p[0] + ditherVal);
        dest->g = uint8_t(p[1] + ditherVal);
        dest->b = uint8_t(p[2] + ditherVal);

    }, parallel);
}

void
gammaAndQuantizeTo8bit(Rgb888Buffer& destBuffer, 
                       const VariablePixelBuffer& srcBuffer,
                       PixelBufferUtilOptions options, 
                       float exposure, float gamma)
{
    switch (srcBuffer.getFormat()) {
    case VariablePixelBuffer::FLOAT:
        gammaAndQuantizeTo8bit(destBuffer, srcBuffer.getFloatBuffer(), options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT2:
        gammaAndQuantizeTo8bit(destBuffer, srcBuffer.getFloat2Buffer(), options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT3:
        gammaAndQuantizeTo8bit(destBuffer, srcBuffer.getFloat3Buffer(), options, exposure, gamma);
        break;
    default:
        MNRY_ASSERT(0 && "unsupported pixel format");
        break;
    }
}

void
gammaAndQuantizeTo8bit(Rgba8888Buffer& destBuffer, 
                       const RenderBuffer& srcBuffer,
                       PixelBufferUtilOptions options, 
                       float exposure, float gamma)
{
    const bool applyGamma = options & PIXEL_BUFFER_UTIL_OPTIONS_APPLY_GAMMA;
    const bool normalize  = options & PIXEL_BUFFER_UTIL_OPTIONS_NORMALIZE;
    const bool parallel   = options & PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL;

    const RenderColor zeroColor(0.0f);
    const RenderColor oneColor(1.0f);

    Vec3f offset(0.f);
    float scale(1.f);
    if (normalize) {
        computeNormalizedScaleAndOffset(offset, scale, srcBuffer);
    }

    processPixelBuffer(destBuffer, srcBuffer, [&](ByteColor4 *dest,
                                                  const RenderColor &src,
                                                  unsigned x, unsigned y) {
        RenderColor p = src;

        if (normalize) {
            p[0] = (p[0] + offset[0]) * scale;
            p[1] = (p[1] + offset[1]) * scale;
            p[2] = (p[2] + offset[2]) * scale;
        } else {
            // Apply exposure
            float exposureScale = pow(2.f, exposure);
            p[0] *= exposureScale;
            p[1] *= exposureScale;
            p[2] *= exposureScale;

            // Apply user gamma
            p[0] = pow(p[0], 1.f / gamma);
            p[1] = pow(p[1], 1.f / gamma);
            p[2] = pow(p[2], 1.f / gamma);

            // Clamp pixels to 0.0 -> 1.0 range.
            p = min(p, oneColor);
            p = max(p, zeroColor);
        }

        if (applyGamma) {
            gammaCorrectColorComponent(p[0]);
            gammaCorrectColorComponent(p[1]);
            gammaCorrectColorComponent(p[2]);
        }

        // Dither and quantize to 8-bit.
        float ditherVal = sDitherMatrix[y & 7][x & 7];
        dest->r = uint8_t(p[0] + ditherVal);
        dest->g = uint8_t(p[1] + ditherVal);
        dest->b = uint8_t(p[2] + ditherVal);
        dest->a = uint8_t(p[3] + ditherVal);

    }, parallel);
}

template<typename SRC_PIXEL_TYPE, typename SRC_TO_CHANNEL>
static void
extractChannelInternal(Rgb888Buffer& destBuffer, 
                       const PixelBuffer<SRC_PIXEL_TYPE>& srcBuffer,
                       SRC_TO_CHANNEL const &s2c,
                       PixelBufferUtilOptions options,
                       float exposure, float gamma)
{
    const bool parallel = options & PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL;

    processPixelBuffer(destBuffer, srcBuffer, [&](ByteColor *dest, const SRC_PIXEL_TYPE &src,
                                                  unsigned x, unsigned y) {
        float gain = pow(2.f, exposure);
        float userGamma = 1.f / gamma;                                       
        uint8_t v = gammaCorrectDitherQuantize(saturate(pow(s2c(src) * gain, userGamma)), x, y);
        dest->r = dest->g = dest->b = v;
    }, parallel);
}

void
extractRedChannel(Rgb888Buffer& destBuffer, 
                  const RenderBuffer& srcBuffer, 
                  PixelBufferUtilOptions options,
                  float exposure, float gamma)
{
    extractChannelInternal(destBuffer, srcBuffer,
                           [](const RenderColor &v) { return v.x; },
                           options, exposure, gamma);
}

void
extractRedChannel(Rgb888Buffer& destBuffer, 
                  const VariablePixelBuffer& srcBuffer, 
                  PixelBufferUtilOptions options,
                  float exposure, float gamma)
{
    switch(srcBuffer.getFormat()) {
    case VariablePixelBuffer::FLOAT:
        extractChannelInternal(destBuffer, srcBuffer.getFloatBuffer(),
                               [](float f) { return f; },
                               options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT2:
        extractChannelInternal(destBuffer, srcBuffer.getFloat2Buffer(),
                               [](const math::Vec2f &v) { return v.x; },
                               options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT3:
        extractChannelInternal(destBuffer, srcBuffer.getFloat3Buffer(),
                               [](const math::Vec3f &v) { return v.x; }, 
                               options, exposure, gamma);
        break;
    default:
        MNRY_ASSERT(0 && "unsupported pixel format");
    }
}


void
extractGreenChannel(Rgb888Buffer& destBuffer, 
                    const RenderBuffer& srcBuffer, 
                    PixelBufferUtilOptions options,
                    float exposure, float gamma)
{
    extractChannelInternal(destBuffer, srcBuffer,
                           [](const RenderColor &v) { return v.y; },
                           options, exposure, gamma);
}

void
extractGreenChannel(Rgb888Buffer& destBuffer, 
                    const VariablePixelBuffer& srcBuffer, 
                    PixelBufferUtilOptions options,
                    float exposure, float gamma)
{
    switch(srcBuffer.getFormat()) {
    case VariablePixelBuffer::FLOAT:
        extractChannelInternal(destBuffer, srcBuffer.getFloatBuffer(),
                               [](float f) { return f; },
                               options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT2:
        extractChannelInternal(destBuffer, srcBuffer.getFloat2Buffer(),
                               [](const math::Vec2f &v) { return v.y; },
                               options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT3:
        extractChannelInternal(destBuffer, srcBuffer.getFloat3Buffer(),
                               [](const math::Vec3f &v) { return v.y; },
                               options, exposure, gamma);
        break;
    default:
        MNRY_ASSERT(0 && "unsupported pixel format");
    }
}


void
extractBlueChannel(Rgb888Buffer& destBuffer, 
                   const RenderBuffer& srcBuffer, 
                   PixelBufferUtilOptions options,
                   float exposure, float gamma)
{
    extractChannelInternal(destBuffer, srcBuffer,
                           [](const RenderColor &v) { return v.z; },
                           options, exposure, gamma);
}

void
extractBlueChannel(Rgb888Buffer& destBuffer, 
                   const VariablePixelBuffer& srcBuffer, 
                   PixelBufferUtilOptions options,
                   float exposure, float gamma)
{
    switch(srcBuffer.getFormat()) {
    case VariablePixelBuffer::FLOAT:
        extractChannelInternal(destBuffer, srcBuffer.getFloatBuffer(),
                               [](float f) { return f; },
                               options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT2:
        extractChannelInternal(destBuffer, srcBuffer.getFloat2Buffer(),
                               [](const math::Vec2f &v) { return 0.f; },
                               options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT3:
        extractChannelInternal(destBuffer, srcBuffer.getFloat3Buffer(),
                               [](const math::Vec3f &v) { return v.z; },
                               options, exposure, gamma);
        break;
    default:
        MNRY_ASSERT(0 && "unsupported pixel format");
    }
}


template<typename SRC_PIXEL_TYPE, typename SRC_TO_ALPHA>
static void
extractAlphaChannelInternal(Rgb888Buffer& destBuffer, 
                            const PixelBuffer<SRC_PIXEL_TYPE>& srcBuffer,
                            SRC_TO_ALPHA const &s2a,
                            PixelBufferUtilOptions options)
{
    const bool parallel = options & PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL;

    processPixelBuffer(destBuffer, srcBuffer, [&](ByteColor *dest, const SRC_PIXEL_TYPE &src,
                                                  unsigned x, unsigned y) {
        uint8_t v = s2a(src);
        dest->r = dest->g = dest->b = v;
    }, parallel);
}

void
extractAlphaChannel(Rgb888Buffer& destBuffer, 
                    const RenderBuffer& srcBuffer, 
                    PixelBufferUtilOptions options,
                    float exposure, float gamma)
{
    extractAlphaChannelInternal(destBuffer, srcBuffer,
                                [exposure, gamma](const RenderColor &v) { return uint8_t(saturate(pow(v.w * pow(2.f, exposure), 1.f / gamma)) * 255.f); },
                                options);
}

void
extractAlphaChannel(Rgb888Buffer& destBuffer, 
                    const VariablePixelBuffer& srcBuffer, 
                    PixelBufferUtilOptions options)
{
    switch(srcBuffer.getFormat()) {
    case VariablePixelBuffer::FLOAT:
        extractAlphaChannelInternal(destBuffer, srcBuffer.getFloatBuffer(),
                                    [](float) { return 0.f; },
                                    options);
        break;
    case VariablePixelBuffer::FLOAT2:
        extractAlphaChannelInternal(destBuffer, srcBuffer.getFloat2Buffer(),
                                    [](const math::Vec2f &) { return 0.f; },
                                    options);
        break;
    case VariablePixelBuffer::FLOAT3:
        extractAlphaChannelInternal(destBuffer, srcBuffer.getFloat3Buffer(),
                                    [](const math::Vec3f &) { return 0.f; },
                                    options);
        break;
    default:
        MNRY_ASSERT(0 && "unsupported pixel format");
    }
}

template<typename SRC_PIXEL_TYPE, typename SRC_TO_COLOR>
static void
extractLuminanceInternal(Rgb888Buffer& destBuffer, 
                         const PixelBuffer<SRC_PIXEL_TYPE>& srcBuffer,
                         SRC_TO_COLOR const &s2c,
                         PixelBufferUtilOptions options,
                         float exposure,
                         float gamma)

{
    const bool parallel = options & PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL;

    processPixelBuffer(destBuffer, srcBuffer, [&](ByteColor *dest, const SRC_PIXEL_TYPE &src,
                                                  unsigned x, unsigned y) {
        float gain = pow(2.f, exposure);
        float userGamma = 1.f / gamma;
        float lum = saturate(pow(luminance(s2c(src)) * gain, userGamma));
        uint8_t v = gammaCorrectDitherQuantize(lum, x, y);
        dest->r = dest->g = dest->b = v;
    }, parallel);
}

void
extractLuminance(Rgb888Buffer& destBuffer, 
                 const RenderBuffer& srcBuffer, 
                 PixelBufferUtilOptions options,
                 float exposure, float gamma)
{
    extractLuminanceInternal(destBuffer, srcBuffer,
                             [](const RenderColor &v) { return math::Color(v.x, v.y, v.z); },
                             options, exposure, gamma);
}

void
extractLuminance(Rgb888Buffer& destBuffer, 
                 const VariablePixelBuffer& srcBuffer, 
                 PixelBufferUtilOptions options,
                 float exposure, float gamma)
{
    switch(srcBuffer.getFormat()) {
    case VariablePixelBuffer::FLOAT:
        extractLuminanceInternal(destBuffer, srcBuffer.getFloatBuffer(),
                                 [](float f) { return math::Color(f, f, f); },
                                 options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT2:
        extractLuminanceInternal(destBuffer, srcBuffer.getFloat2Buffer(),
                                 [](const math::Vec2f &v) { return math::Color(v.x, v.y, 0.f); },
                                 options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT3:
        extractLuminanceInternal(destBuffer, srcBuffer.getFloat3Buffer(),
                                 [](const math::Vec3f &v) { return math::Color(v.x, v.y, v.z); },
                                 options, exposure, gamma);
        break;
    default:
        MNRY_ASSERT(0 && "unsupported pixel format");
    }
}

template<typename BUFFER_T>
static void
extractSaturationInternal(Rgb888Buffer& destBuffer, 
                          const BUFFER_T& srcBuffer, 
                          PixelBufferUtilOptions options,
                          float exposure, float gamma)
{
    // TODO... this is just a temp copy in the meantime.
    options |= PIXEL_BUFFER_UTIL_OPTIONS_APPLY_GAMMA;
    gammaAndQuantizeTo8bit(destBuffer, srcBuffer, options, exposure, gamma);
}

void
extractSaturation(Rgb888Buffer& destBuffer, 
                  const RenderBuffer& srcBuffer, 
                  PixelBufferUtilOptions options,
                  float exposure, float gamma)
{
    extractSaturationInternal(destBuffer, srcBuffer, options, exposure, gamma);
}

void
extractSaturation(Rgb888Buffer& destBuffer, 
                  const VariablePixelBuffer& srcBuffer, 
                  PixelBufferUtilOptions options,
                  float exposure, float gamma)
{
    switch(srcBuffer.getFormat()) {
    case VariablePixelBuffer::FLOAT:
        extractSaturationInternal(destBuffer, srcBuffer.getFloatBuffer(), options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT2:
        extractSaturationInternal(destBuffer, srcBuffer.getFloat2Buffer(), options, exposure, gamma);
        break;
    case VariablePixelBuffer::FLOAT3:
        extractSaturationInternal(destBuffer, srcBuffer.getFloat3Buffer(), options, exposure, gamma);
        break;
    default:
        MNRY_ASSERT(0 && "unsupported pixel format");
    }
}

void
visualizeSamplesPerPixel(scene_rdl2::fb_util::Rgb888Buffer &destBuffer,
                        const scene_rdl2::fb_util::FloatBuffer &samplesPerPixel,
                        bool parallel)
{
    unsigned w = samplesPerPixel.getWidth();
    unsigned h = samplesPerPixel.getHeight();

    destBuffer.init(w, h);

    simpleLoop(parallel, 0u, h, [&](unsigned y) {

        scene_rdl2::fb_util::ByteColor *dst = destBuffer.getRow(y);
        const float *src = samplesPerPixel.getRow(y);

        for (unsigned x = 0; x < w; ++x) {
            float s = clamp(*src, 0.f, 255.f);
            dst->r = dst->g = dst->b = uint8_t(s);
            ++dst;
            ++src;
        }
    });
}

} // namespace fb_util
} // namespace scene_rdl2

