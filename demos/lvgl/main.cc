/**
 * @file main.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* needed for usleep() */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <textra/fontmgr_collection.h>

#include "textra/layout_drawer.h"
#include "textra/platform/lvgl/lvgl_canvas_helper.h"
#include "textra/text_layout.h"
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif
#include <SDL.h>
#include <textra/paragraph.h>

#include "hal/hal.h"
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

#define CANVAS_WIDTH 300
#define CANVAS_HEIGHT 200

using namespace tttext;
static void timer_cb(lv_timer_t* timer) {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  Style run_style;
  float base_size = 24;
  std::string content = "hello world~~ 中国人";
  run_style.SetTextSize(base_size);
  run_style.SetForegroundColor(TTColor::RED);
  paragraph.AddTextRun(&run_style, content.c_str(),
                       static_cast<uint32_t>(content.length()));

  FontmgrCollection collection;
  TextLayout layout(&collection);
  auto page_ptr = std::make_unique<LayoutRegion>(
      CANVAS_WIDTH, CANVAS_HEIGHT, LayoutMode::kAtMost, LayoutMode::kAtMost);
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  auto& page = *page_ptr;
  layout.Layout(&paragraph, &page, context);

  lv_obj_t* canvas = (lv_obj_t*)lv_timer_get_user_data(timer);
  lv_layer_t layer;
  lv_canvas_init_layer(canvas, &layer);

  lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER);

  LVGLCanvasHelper canvas_helper(&layer);

  LayoutDrawer drawer(&canvas_helper);
  drawer.DrawLayoutPage(&page);

  lv_canvas_finish_layer(canvas, &layer);
}

void lv_example_canvas_11(void) {
  /*Create a buffer for the canvas*/
  LV_DRAW_BUF_DEFINE_STATIC(draw_buf, CANVAS_WIDTH, CANVAS_HEIGHT,
                            LV_COLOR_FORMAT_ARGB8888);
  LV_DRAW_BUF_INIT_STATIC(draw_buf);

  lv_obj_t* canvas = lv_canvas_create(lv_screen_active());
  lv_obj_set_size(canvas, CANVAS_WIDTH, CANVAS_HEIGHT);

  lv_obj_center(canvas);

  lv_canvas_set_draw_buf(canvas, &draw_buf);

  lv_timer_create(timer_cb, 16, canvas);
}

#if LV_USE_OS != LV_OS_FREERTOS

int main(int argc, char** argv) {
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  sdl_hal_init(320, 480);

  /* Run the default demo */
  /* To try a different demo or example, replace this with one of: */
  /* - lv_demo_benchmark(); */
  /* - lv_demo_stress(); */
  /* - lv_example_label_1(); */
  /* - etc. */
  lv_example_canvas_11();
  // lv_demo_widgets();

  while (1) {
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    uint32_t sleep_time_ms = lv_timer_handler();
    if (sleep_time_ms == LV_NO_TIMER_READY) {
      sleep_time_ms = LV_DEF_REFR_PERIOD;
    }
#ifdef _MSC_VER
    Sleep(sleep_time_ms);
#else
    usleep(sleep_time_ms * 1000);
#endif
  }

  return 0;
}

#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/
