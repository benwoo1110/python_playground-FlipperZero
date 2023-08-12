#include "python_playground.h"

bool connected_input_callback(InputEvent* input_event, void* ctx) {
    FURI_LOG_D(LOG_TAG, "Connected input");
    PPApp_t* app = ctx;

    if (input_event->key == InputKeyBack) {
        app->cli_running = false;
        FURI_LOG_D(LOG_TAG, "Connected input back");
        return true;
    }

    InputData_t* input_data = malloc(sizeof(InputData_t));
    input_data->key = input_event->key;
    input_data->type = input_event->type;
    protocol_send(app->cli, INPUT_ID, input_data);
    free(input_data);
    return true;
}

void connected_draw_callback(Canvas* canvas, void* ctx) {
    ConnectedViewModel_t* vm = ctx;

    if (vm->draw_data == NULL) {
        return;
    }

    canvas_clear(canvas);
    for (int i = 0; i < vm->draw_data->draw_arr_size; i++) {
        ProtocolData_t* draw_element = &vm->draw_data->draw_arr[i];
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
            case GUI_DRAW_ICON_ID: {
                GuiDrawIconData_t* draw_icon_data = (GuiDrawIconData_t*)draw_element->data;
                Icon** iconp = IconDict_get(vm->icons, draw_icon_data->icon_id);
                if (iconp == NULL) {
                    FURI_LOG_E(LOG_TAG, "Icon not found");
                    break;
                }
                canvas_draw_icon(canvas, draw_icon_data->x, draw_icon_data->y, *iconp);
                break;
            }
            case GUI_DRAW_DOT_ID: {
                GuiDrawDotData_t* draw_dot_data = (GuiDrawDotData_t*)draw_element->data;
                canvas_draw_dot(canvas, draw_dot_data->x, draw_dot_data->y);
                break;
            }
            case GUI_DRAW_LINE_ID: {
                GuiDrawLineData_t* draw_line_data = (GuiDrawLineData_t*)draw_element->data;
                canvas_draw_line(canvas, draw_line_data->x1, draw_line_data->y1, draw_line_data->x2, draw_line_data->y2);
                break;
            }
            case GUI_DRAW_CIRCLE_ID: {
                GuiDrawCircleData_t* draw_circle_data = (GuiDrawCircleData_t*)draw_element->data;
                canvas_draw_circle(canvas, draw_circle_data->x, draw_circle_data->y, draw_circle_data->radius);
                break;
            }
            case GUI_DRAW_DISC_ID: {
                GuiDrawDiscData_t* draw_disc_data = (GuiDrawDiscData_t*)draw_element->data;
                canvas_draw_disc(canvas, draw_disc_data->x, draw_disc_data->y, draw_disc_data->radius);
                break;
            }
            case GUI_DRAW_TRIANGLE_ID: {
                GuiDrawTriangleData_t* draw_triangle_data = (GuiDrawTriangleData_t*)draw_element->data;
                canvas_draw_triangle(canvas, draw_triangle_data->x, draw_triangle_data->y, draw_triangle_data->base, draw_triangle_data->height, draw_triangle_data->direction);
                break;
            }
            case GUI_DRAW_GLYPH_ID: {
                GuiDrawGlyphData_t* draw_glyph_data = (GuiDrawGlyphData_t*)draw_element->data;
                canvas_draw_glyph(canvas, draw_glyph_data->x, draw_glyph_data->y, draw_glyph_data->glyph_char);
                break;
            }
            case GUI_DRAW_BOX_ID: {
                GuiDrawBoxData_t* draw_box_data = (GuiDrawBoxData_t*)draw_element->data;
                canvas_draw_box(canvas, draw_box_data->x, draw_box_data->y, draw_box_data->width, draw_box_data->height);
                break;
            }
            case GUI_DRAW_RBOX_ID: {
                GuiDrawRBoxData_t* draw_rbox_data = (GuiDrawRBoxData_t*)draw_element->data;
                canvas_draw_rbox(canvas, draw_rbox_data->x, draw_rbox_data->y, draw_rbox_data->width, draw_rbox_data->height, draw_rbox_data->radius);
                break;
            }
            case GUI_SET_COLOR_ID: {
                GuiSetColorData_t* set_color_data = (GuiSetColorData_t*)draw_element->data;
                canvas_set_color(canvas, set_color_data->color);
                break;
            }
            case GUI_SET_COLOR_INVERTED_ID: {
                canvas_invert_color(canvas);
                break;
            }
            case GUI_SET_FONT_ID: {
                GuiSetFontData_t* set_font_data = (GuiSetFontData_t*)draw_element->data;
                canvas_set_font(canvas, set_font_data->font);
                break;
            }
            case GUI_SET_FONT_DIRECTION_ID: {
                GuiSetFontDirectionData_t* set_font_direction_data = (GuiSetFontDirectionData_t*)draw_element->data;
                canvas_set_font_direction(canvas, set_font_direction_data->direction);
                break;
            }
            default:
                break;
        }
    }

    if (vm->debug_msg != NULL) {
        canvas_draw_str_aligned(canvas, 5, 15, AlignLeft, AlignCenter, vm->debug_msg);
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
