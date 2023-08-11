#include "protocol.h"

void protocol_payload_free(ProtocolPayload_t* payload) {
    if (payload == NULL) {
        return;
    }
    if (payload->data != NULL) {
        free(payload->data);
    }
    free(payload);
}

void protocol_data_free(ProtocolData_t* data) {
    if (data == NULL) {
        return;
    }
    if (data->data != NULL) {
        free(data->data);
    }
    free(data);
}

ProtocolData_t* protocol_receive(Cli* cli, uint32_t timeout_ms) {
    uint16_t id;
    uint32_t data_size;
    void* data = NULL;
    if (cli_read_timeout(cli, (uint8_t*)&id, sizeof(uint16_t), timeout_ms) < sizeof(uint16_t)) {
        return NULL;
    }
    if (cli_read_timeout(cli, (uint8_t*)&data_size, sizeof(uint32_t), timeout_ms) < sizeof(uint32_t)) {
        return NULL;
    }
    if (data_size > 0) {
        uint8_t* raw_data = malloc(data_size);
        if (cli_read_timeout(cli, raw_data, data_size, timeout_ms) < data_size) {
            free(raw_data);
            return NULL;
        }
        data = protocol_decode(id, data_size, raw_data);
        free(raw_data);
    }

    ProtocolData_t* decoded_data = malloc(sizeof(ProtocolData_t));
    decoded_data->id = id;
    decoded_data->data_size = data_size;
    decoded_data->data = data;
    return decoded_data;
}

void protocol_send_empty(Cli* cli, uint16_t id) {
    cli_write(cli, (uint8_t*)&id, sizeof(id));
    uint32_t data_size = 0;
    cli_write(cli, (uint8_t*)&data_size, sizeof(data_size));
}

void protocol_send(Cli* cli, uint16_t id, void* data) {
    ProtocolPayload_t* payload = protocol_encode(id, data);
    if (payload == NULL) {
        return;
    }
    cli_write(cli, (uint8_t*)&payload->id, sizeof(payload->id));
    cli_write(cli, (uint8_t*)&payload->data_size, sizeof(payload->data_size));
    cli_write(cli, payload->data, payload->data_size);
    protocol_payload_free(payload);
}

void uint8_decode(uint8_t** data, uint8_t* out) {
    *out = **data;
    *data += sizeof(uint8_t);
}

void uint16_decode(uint8_t** data, uint16_t* out) {
    *out = *(uint16_t*)*data;
    *data += sizeof(uint16_t);
}

void uint32_decode(uint8_t** data, uint32_t* out) {
    *out = *(uint32_t*)*data;
    *data += sizeof(uint32_t);
}

void float_decode(uint8_t** data, float* out) {
    *out = *(float*)*data;
    *data += sizeof(float);
}

void bytes_decode(uint8_t** data, uint8_t** out) {
    uint32_t bytes_size;
    uint32_decode(data, &bytes_size);
    *out = malloc(bytes_size);
    memcpy(*out, *data, bytes_size);
    *data += bytes_size;
}

void str_decode(uint8_t** data, char** out) {
    uint32_t str_size;
    uint32_decode(data, &str_size);
    *out = malloc(str_size + 1);
    memcpy(*out, *data, str_size);
    (*out)[str_size] = '\0';
    *data += str_size;
}

void* gui_draw_decode(uint8_t* data) {
    GuiDrawData_t* draw_data = malloc(sizeof(GuiDrawData_t));
    uint16_decode(&data, &draw_data->draw_arr_size);
    draw_data->draw_arr = malloc(sizeof(ProtocolData_t) * draw_data->draw_arr_size);

    for (uint16_t i = 0; i < draw_data->draw_arr_size; i++) {
        ProtocolData_t* draw_element = &draw_data->draw_arr[i];
        uint16_decode(&data, &draw_element->id);
        uint32_decode(&data, &draw_element->data_size);
        draw_element->data = protocol_decode(draw_element->id, draw_element->data_size, data);
        data += draw_element->data_size;
    }

    return draw_data;
}

void* gui_draw_str_decode(uint8_t* data) {
    GuiDrawStrData_t* draw_data = malloc(sizeof(GuiDrawStrData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    str_decode(&data, &draw_data->str);
    return draw_data;
}

void* gui_draw_str_align_decode(uint8_t* data) {
    GuiDrawStrAlignData_t* draw_data = malloc(sizeof(GuiDrawStrAlignData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->horizontal);
    uint8_decode(&data, &draw_data->vertical);
    str_decode(&data, &draw_data->str);
    return draw_data;
}

void* gui_draw_frame_decode(uint8_t* data) {
    GuiDrawFrameData_t* draw_data = malloc(sizeof(GuiDrawFrameData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->width);
    uint8_decode(&data, &draw_data->height);
    return draw_data;
}

void* gui_draw_rframe_decode(uint8_t* data) {
    GuiDrawRFrameData_t* draw_data = malloc(sizeof(GuiDrawRFrameData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->width);
    uint8_decode(&data, &draw_data->height);
    uint8_decode(&data, &draw_data->radius);
    return draw_data;
}

void* gui_draw_icon_decode(uint8_t* data) {
    GuiDrawIconData_t* draw_data = malloc(sizeof(GuiDrawIconData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->icon_id);
    return draw_data;
}

void* gui_draw_dot_decode(uint8_t* data) {
    GuiDrawDotData_t* draw_data = malloc(sizeof(GuiDrawDotData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    return draw_data;
}

void* gui_draw_line_decode(uint8_t* data) {
    GuiDrawLineData_t* draw_data = malloc(sizeof(GuiDrawLineData_t));
    uint8_decode(&data, &draw_data->x1);
    uint8_decode(&data, &draw_data->y1);
    uint8_decode(&data, &draw_data->x2);
    uint8_decode(&data, &draw_data->y2);
    return draw_data;
}

void* gui_draw_circle_decode(uint8_t* data) {
    GuiDrawCircleData_t* draw_data = malloc(sizeof(GuiDrawCircleData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->radius);
    return draw_data;
}

void* gui_draw_disc_decode(uint8_t* data) {
    GuiDrawDiscData_t* draw_data = malloc(sizeof(GuiDrawDiscData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->radius);
    return draw_data;
}

void* gui_draw_triangle_decode(uint8_t* data) {
    GuiDrawTriangleData_t* draw_data = malloc(sizeof(GuiDrawTriangleData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->base);
    uint8_decode(&data, &draw_data->height);
    uint8_decode(&data, &draw_data->direction);
    return draw_data;
}

void* gui_draw_glyph_decode(uint8_t* data) {
    GuiDrawGlyphData_t* draw_data = malloc(sizeof(GuiDrawGlyphData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint16_decode(&data, &draw_data->glyph_char);
    return draw_data;
}

void* gui_draw_box_decode(uint8_t* data) {
    GuiDrawBoxData_t* draw_data = malloc(sizeof(GuiDrawBoxData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->width);
    uint8_decode(&data, &draw_data->height);
    return draw_data;
}

void* gui_draw_rbox_decode(uint8_t* data) {
    GuiDrawRBoxData_t* draw_data = malloc(sizeof(GuiDrawRBoxData_t));
    uint8_decode(&data, &draw_data->x);
    uint8_decode(&data, &draw_data->y);
    uint8_decode(&data, &draw_data->width);
    uint8_decode(&data, &draw_data->height);
    uint8_decode(&data, &draw_data->radius);
    return draw_data;
}

void* gui_icon_add_decode(uint8_t* data) {   
    GuiIconAddData_t* icon_data = malloc(sizeof(GuiIconAddData_t));
    uint8_decode(&data, &icon_data->icon_id);
    uint8_decode(&data, &icon_data->width);
    uint8_decode(&data, &icon_data->height);
    bytes_decode(&data, &icon_data->frame_data);
    return icon_data;
}

void* speaker_play_decode(uint8_t* data) {
    SpeakerPlayData_t* play_data = malloc(sizeof(SpeakerPlayData_t));
    float_decode(&data, &play_data->frequency);
    float_decode(&data, &play_data->volume);
    return play_data;
}

void* speaker_set_volume_decode(uint8_t* data) {
    SpeakerSetVolumeData_t* volume_data = malloc(sizeof(SpeakerSetVolumeData_t));
    float_decode(&data, &volume_data->volume);
    return volume_data;
}

void* light_set_decode(uint8_t* data) {
    LightSetData_t* light_data = malloc(sizeof(LightSetData_t));
    uint8_decode(&data, &light_data->light);
    uint8_decode(&data, &light_data->value);
    return light_data;
}

void* light_sequence_decode(uint8_t* data) {
    LightSequenceData_t* sequence_data = malloc(sizeof(LightSequenceData_t));
    str_decode(&data, &sequence_data->sequence);
    return sequence_data;
}

void* protocol_decode(uint16_t id, uint32_t data_size, uint8_t* data) {
    UNUSED(data_size);
    switch (id) {
        case GUI_DRAW_ID: return gui_draw_decode(data);
        case GUI_DRAW_STR_ID: return gui_draw_str_decode(data);
        case GUI_DRAW_STR_ALIGN_ID: return gui_draw_str_align_decode(data);
        case GUI_DRAW_FRAME_ID: return gui_draw_frame_decode(data);
        case GUI_DRAW_RFRAME_ID: return gui_draw_rframe_decode(data);
        case GUI_DRAW_ICON_ID: return gui_draw_icon_decode(data);
        case GUI_DRAW_DOT_ID: return gui_draw_dot_decode(data);
        case GUI_DRAW_LINE_ID: return gui_draw_line_decode(data);
        case GUI_DRAW_CIRCLE_ID: return gui_draw_circle_decode(data);
        case GUI_DRAW_DISC_ID: return gui_draw_disc_decode(data);
        case GUI_DRAW_TRIANGLE_ID: return gui_draw_triangle_decode(data);
        case GUI_DRAW_GLYPH_ID: return gui_draw_glyph_decode(data);
        case GUI_DRAW_BOX_ID: return gui_draw_box_decode(data);
        case GUI_DRAW_RBOX_ID: return gui_draw_rbox_decode(data);

        case GUI_ICON_ADD_ID: return gui_icon_add_decode(data);

        case HW_SPEAKER_PLAY_ID: return speaker_play_decode(data);
        case HW_SPEAKER_SET_VOLUME_ID: return speaker_set_volume_decode(data);
        case HW_LIGHT_SET_ID: return light_set_decode(data);
        case HW_LIGHT_SEQUENCE_ID: return light_sequence_decode(data);

        default: return NULL;
    }
}

ProtocolPayload_t* input_encode(uint16_t id, void* data) {
    InputData_t* input_data = data;
    ProtocolPayload_t* payload = malloc(sizeof(ProtocolPayload_t));
    payload->id = id;
    payload->data_size = 2;
    payload->data = malloc(payload->data_size);
    payload->data[0] = input_data->key;
    payload->data[1] = input_data->type;
    return payload;
}

ProtocolPayload_t* protocol_encode(uint16_t id, void* data) {
    switch (id) {
        case INPUT_ID: return input_encode(id, data);
        default: return NULL;
    }
}
