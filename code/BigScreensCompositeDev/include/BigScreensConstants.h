//
//  BigScreensConstants.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/25/13.
//
//

#pragma once

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
const static float kPosColumn2 = (3.5f + 33.166f) * kPxPerFoot;
const static float kPosColumn3 = (3.5f + 33.166f + 31.8333f) * kPxPerFoot;
const static float kPosColumn4 = (3.5f + 33.166f + 31.8333f + 33.33333f) * kPxPerFoot;

// Audio
const static int kNumFFTChannels = 64;