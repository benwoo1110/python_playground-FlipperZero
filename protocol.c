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
    uint8_t* data;
    if (cli_read_timeout(cli, (uint8_t*)&id, sizeof(uint16_t), timeout_ms) < sizeof(uint16_t)) {
        return NULL;
    }
    if (cli_read_timeout(cli, (uint8_t*)&data_size, sizeof(uint32_t), timeout_ms) < sizeof(uint32_t)) {
        return NULL;
    }
    data = malloc(data_size);
    if (cli_read_timeout(cli, data, data_size, timeout_ms) < data_size) {
        free(data);
        return NULL;
    }
    ProtocolData_t* decoded_data = malloc(sizeof(ProtocolData_t));
    decoded_data->id = id;
    decoded_data->data_size = data_size;
    decoded_data->data = protocol_decode(id, data_size, data);
    free(data);
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

void* protocol_decode(uint16_t id, uint32_t data_size, uint8_t* data) {
    UNUSED(data_size);
    switch (id) {
        case GUI_DRAW_ID: return gui_draw_decode(data);
        case GUI_DRAW_STR_ID: return gui_draw_str_decode(data);
        case GUI_DRAW_STR_ALIGN_ID: return gui_draw_str_align_decode(data);
        case GUI_DRAW_FRAME_ID: return gui_draw_frame_decode(data);
        case GUI_DRAW_RFRAME_ID: return gui_draw_rframe_decode(data);
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
