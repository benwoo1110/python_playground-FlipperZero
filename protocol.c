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
    //free(data);
    return decoded_data;
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

void* gui_draw_decode(uint8_t* data) {
    GuiDrawData_t* draw_data = malloc(sizeof(GuiDrawData_t));
    draw_data->draw_arr_size = *(uint16_t*)data;
    draw_data->draw_arr = malloc(sizeof(ProtocolData_t) * draw_data->draw_arr_size);
    
    uint8_t* data_offset = data + sizeof(uint16_t);
    for (uint16_t i = 0; i < draw_data->draw_arr_size; i++) {
        uint16_t id = *(uint16_t*)(data_offset);
        data_offset += sizeof(uint16_t);

        uint32_t data_size = *(uint32_t*)data_offset;
        data_offset += sizeof(uint32_t);

        uint8_t* raw_data = data_offset;
        data_offset += data_size;

        ProtocolData_t* one_draw_data = &draw_data->draw_arr[i];

        // ProtocolData_t* one_draw_data = draw_data->draw_arr + (i * sizeof(ProtocolData_t));
        one_draw_data->id = id;
        one_draw_data->data_size = data_size;
        one_draw_data->data = protocol_decode(id, data_size, raw_data);
        //UNUSED(one_draw_data);
        UNUSED(raw_data);
    }

    return draw_data;
}

void* gui_draw_str_decode(uint8_t* data, uint32_t data_size) {
    GuiDrawStrData_t* draw_data = malloc(sizeof(GuiDrawStrData_t));
    draw_data->x = data[0];
    draw_data->y = data[1];
    draw_data->str = malloc(data_size - 2);
    memcpy(draw_data->str, data + 2, data_size - 2);
    return draw_data;
}

void* gui_draw_frame_decode(uint8_t* data) {
    GuiDrawFrameData_t* draw_data = malloc(sizeof(GuiDrawFrameData_t));
    draw_data->x = data[0];
    draw_data->y = data[1];
    draw_data->width = data[2];
    draw_data->height = data[3];

    return draw_data;
}

void* protocol_decode(uint16_t id, uint32_t data_size, uint8_t* data) {
    switch (id) {
        case GUI_DRAW_ID: return gui_draw_decode(data);
        case GUI_DRAW_STR_ID: return gui_draw_str_decode(data, data_size);
        case GUI_DRAW_FRAME_ID: return gui_draw_frame_decode(data);
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
