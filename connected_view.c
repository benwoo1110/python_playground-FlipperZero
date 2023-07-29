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
    free(input_data);
    furi_mutex_release(app->mutex);
    return true;
}

void connected_draw_callback(Canvas* canvas, void* ctx) {
    ConnectedViewModel_t* vm = ctx;

    canvas_clear(canvas);
    // if (vm->debug_msg != NULL) {
    //     canvas_draw_str_aligned(canvas, 5, 5, AlignCenter, AlignCenter, vm->debug_msg);
    // }
    
    if (vm->draw_data == NULL) {
        return;
    }

    // char aa[64];
    // snprintf(aa, 64, "%u", vm->draw_data->id_test);
    // canvas_draw_str_aligned(canvas, 20, 5, AlignCenter, AlignCenter, aa);

    for (int i = 0; i < vm->draw_data->draw_arr_size; i++) {
        ProtocolData_t* draw_element = &vm->draw_data->draw_arr[i];

        // char str[16];
        // snprintf(str, 16, "%lu", draw_element->data_size);
        // canvas_draw_str_aligned(canvas, 20, 10+(i*10), AlignCenter, AlignCenter, str);
        
        switch (draw_element->id) {
            case GUI_DRAW_STR_ID: {
                GuiDrawStrData_t* draw_str_data = (GuiDrawStrData_t*)draw_element->data;
                canvas_draw_str(canvas, draw_str_data->x, draw_str_data->y, draw_str_data->str);
                break;
            }
            case GUI_DRAW_STR_ALIGN_ID: {
                GuiDrawStrAlignData_t* draw_str_align_data = (GuiDrawStrAlignData_t*)draw_element->data;
                canvas_draw_str_aligned(canvas, draw_str_align_data->x, draw_str_align_data->y, draw_str_align_data->horizontal, draw_str_align_data->vertical, draw_str_align_data->str);
                break;
            }
            case GUI_DRAW_FRAME_ID: {
                GuiDrawFrameData_t* draw_frame_data = (GuiDrawFrameData_t*)draw_element->data;
                canvas_draw_frame(canvas, draw_frame_data->x, draw_frame_data->y, draw_frame_data->width, draw_frame_data->height);
                break;
            }
            case GUI_DRAW_RFRAME_ID: {
                GuiDrawRFrameData_t* draw_rframe_data = (GuiDrawRFrameData_t*)draw_element->data;
                canvas_draw_rframe(canvas, draw_rframe_data->x, draw_rframe_data->y, draw_rframe_data->width, draw_rframe_data->height, draw_rframe_data->radius);
                break;
            }
            default:
                break;    
        }
    }
}

View* connected_view_alloc(PPApp_t* app) {
    FURI_LOG_I(LOG_TAG, "Connected view init");
    View* view = view_alloc();
    view_set_context(view, app);
    view_set_input_callback(view, connected_input_callback);
    view_set_draw_callback(view, connected_draw_callback);
    view_allocate_model(view, ViewModelTypeLocking, sizeof(ConnectedViewModel_t));
    view_dispatcher_add_view(app->view_dispatcher, CONNECTED_VIEW_ID, view);
    return view;
}
