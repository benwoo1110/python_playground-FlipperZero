#include "python_playground.h"

bool connected_input_callback(InputEvent* input_event, void* ctx) {
    FURI_LOG_D(LOG_TAG, "Connected input");
    PPApp_t* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);

    if (input_event->key == InputKeyBack) {
        app->cli_running = false;
        FURI_LOG_D(LOG_TAG, "Connected input back");
        furi_mutex_release(app->mutex);
        return true;
    }

    InputData_t* input_data = malloc(sizeof(InputData_t));
    input_data->key = input_event->key;
    input_data->type = input_event->type;
    protocol_send(app->cli, INPUT_ID, input_data);

    furi_mutex_release(app->mutex);
    return true;
}

void connected_draw_callback(Canvas* canvas, void* ctx) {
    ConnectedViewModel_t* vm = ctx;
    canvas_clear(canvas);
    if (vm->draw_data != NULL) {
        canvas_draw_str_aligned(
            canvas, 
            vm->draw_data->x, 
            vm->draw_data->y, 
            AlignCenter, 
            AlignCenter,
            vm->draw_data->str);
    }
}

View* connected_view_alloc(PPApp_t* app) {
    FURI_LOG_I(LOG_TAG, "Connected view init");
    View* view = view_alloc();
    view_set_context(view, app);
    view_set_input_callback(view, connected_input_callback);
    view_set_draw_callback(view, connected_draw_callback);
    view_allocate_model(view, ViewModelTypeLockFree, sizeof(ConnectedViewModel_t));
    view_dispatcher_add_view(app->view_dispatcher, CONNECTED_VIEW_ID, view);
    return view;
}
