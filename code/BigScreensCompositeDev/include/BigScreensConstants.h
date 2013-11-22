//
//  BigScreensConstants.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/25/13.
//
//

#pragma once

// Content Keys
static const std::string kContentKeyTankSpin = "tankSpin";
static const std::string kContentKeyTankOverhead = "tankOverhead";
static const std::string kContentKeyTankHeightmap = "tankHeightmap";
static const std::string kContentKeyTankFlat = "tankFlat";
static const std::string kContentKeyTankWide = "tankWide";
static const std::string kContentKeyTankSideCarriage = "tankSide";
static const std::string kContentKeyTankHorizon = "tankHorizon";
static const std::string kContentKeyTankMultiOverhead = "tankMultiOver";
static const std::string kContentKeyTextPrefix = "text";
static const std::string kContentKeyTanksConverge = "tanksConverge";
static const std::string kContentKeySingleTankConverge = "singleTankConverge";
static const std::string kContentKeyPerlin = "perlin";
static const std::string kContentKeyOpponent = "opponent";
static const std::string kContentKeyStatic = "static";

// MPE Messages
const static std::string kMPEMessagePlay = "play";
const static std::string kMPEMessagePause = "pause";
const static std::string kMPEMessagePrev = "prev";
const static std::string kMPEMessageNext = "next";
const static std::string kMPEMessageLoad = "load";
const static std::string kMPEMessageRestart = "restart";
const static std::string kMPEMessageShowColumns = "showColumns";
const static std::string kMPEMessageHideColumns = "hideColumns";
const static std::string kMPEMessageCurrentLayout = "layout";
const static std::string kMPEMessageDelimeter = "~";

// Screen Info
const static int kNumScreens = 3;
const static int kScreenWidth = 3840;
const static int kScreenHeight = 1080;
const static int kScreenMarginBottom = 133;
const static int kScreenMarginRight = 147; // Estimate

#if IS_IAC
const static float kScreenScale = 1.0f;
#else
const static float kScreenScale = 0.25f;
#endif

// Columns
const static float kScreenWidthFeet = 120.0f;
const static float kPxPerFoot = (float)((kScreenWidth * kNumScreens) - kScreenMarginRight) / kScreenWidthFeet;
const static float kColumnWidth = 3.0f * kPxPerFoot;
const static float kPosColumn1 = 3.5f * kPxPerFoot;
const static float kPosColumn2 = (3.5f + 34.166f) * kPxPerFoot;
const static float kPosColumn3 = (3.5f + 34.166f + 32.8333f) * kPxPerFoot;
const static float kPosColumn4 = (3.5f + 34.166f + 32.8333f + 34.33333f) * kPxPerFoot;

// Audio
const static int kNumFFTChannels = 64;

// Timing
// This should be synchronized w/ the timeline.
const long kMSFullConvergence = 30000; // 30 sec
extern long MSCamerasConverge;// TBD: Determined by the layout timeline
extern long MSConvergeBeforeCameraMerge;// TBD: Determined by the layout timeline
// ... was kMSFullConvergence - kMSCamerasConverge;
// The last layout must come kNumFramesConvergeBeforeCameraMerge frames after the second to last

// Model Measurements
const static float kTankBarrelTipZ = 470.0f;
const static float kTankBarrelTipY = 164.0f;
const static float kTankBarrelTipX = 0.0f;

const static float kTankBodyWidth = 360.0f;
const static float kTankBodyHeadToToe = 670.0f;
const static float kTankCenterZOffset = 0.597;

const static float kTankHeadOffsetZ = 65.0f;
const static float kTankBarrelOffsetY = 170.0f;
const static float kTankBarrelLength = 461.0f;

const static float kTankBodyFrontZ = 267.0f;
const static float kTankBodyCenterY = 107.0f;

// Scale
const static float kOpponentScale = 1000.0f;
const static float kMinionScale = 150.0f;
