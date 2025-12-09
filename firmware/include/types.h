 #pragma once

typedef enum {
  MODE_CLOCK,        // Default - shows time/date
  MODE_CONVERSATION, // Active voice conversation
  MODE_SETUP         // First boot setup mode
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
