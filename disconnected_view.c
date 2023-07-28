#include "python_playground.h"

bool disconnected_input_callback(InputEvent* input_event, void* ctx) {
    FURI_LOG_D(LOG_TAG, "Disconnected input");
    PPApp_t* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

    if (input_event->key == InputKeyBack && input_event->type == InputTypeShort) {
        view_dispatcher_stop(app->view_dispatcher);
    }
    FURI_LOG_D(LOG_TAG, "Input: %s %s", input_get_key_name(input_event->key), input_get_type_name(input_event->type));
    furi_mutex_release(app->mutex);
    return true;
}

void disconnected_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignCenter, "Connect CLI to start...");
    canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignCenter, "Press Back to exit");
}

View* disconnected_view_alloc(PPApp_t* app) {
    FURI_LOG_I(LOG_TAG, "Disconnected view init");
    View* view = view_alloc();
    view_set_context(view, app);
    view_set_input_callback(view, disconnected_input_callback);
    view_set_draw_callback(view, disconnected_draw_callback);
    view_dispatcher_add_view(app->view_dispatcher, DISCONNECTED_VIEW_ID, view);
    return view;
}
