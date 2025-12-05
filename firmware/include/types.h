 #pragma once

typedef enum {
  MODE_TIME_DATE,

  MODE_CHAT,
  MODE_WIFI_INFO
} DisplayMode_t;

typedef enum {
  STATE_BOOT,
  STATE_IDLE,
  STATE_LISTENING,
  STATE_THINKING,
  STATE_SPEAKING,
  STATE_PLAYING,
  STATE_ERROR
} RobotState_t;

typedef enum {
  EXPR_NORMAL,
  EXPR_HAPPY,
  EXPR_SAD,
  EXPR_THINKING,
  EXPR_LOGO
} Expression_t;

typedef enum {
  THEME_DEFAULT,   // Elaborate theme with date bars
  THEME_COMPACT    // Simple compact theme
} DisplayTheme_t;
