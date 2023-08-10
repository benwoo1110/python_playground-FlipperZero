#include "python_playground.h"

// ##############
// CLI
void run_data(PPApp_t* app, ProtocolData_t* proto_data) {
    switch (proto_data->id) {
        case GUI_DRAW_ID: {
            ConnectedViewModel_t* vm = view_get_model(app->connected_view);
            if (vm->draw_data != NULL) {
                for (uint16_t i = 0; i < vm->draw_data->draw_arr_size; i++) {
                    protocol_data_free(&vm->draw_data->draw_arr[i]);
                }
                free(vm->draw_data->draw_arr);
                free(vm->draw_data);
                vm->draw_data = NULL;
            }
            vm->draw_data = malloc(sizeof(GuiDrawData_t));
            memcpy(vm->draw_data, proto_data->data, sizeof(GuiDrawData_t));
            view_commit_model(app->connected_view, true);
            break;
        }
        case GUI_ICON_ADD_ID: {
            GuiIconAddData_t* icon_add_data = proto_data->data;

            const uint8_t** frames = malloc(sizeof(uint8_t*));
            frames[0] = icon_add_data->frame_data;

            Icon icon = {
                .width = icon_add_data->width,
                .height = icon_add_data->height,
                .frame_count = 1,
                .frame_rate = 0,
                .frames = frames,
            };

            Icon* iconp = malloc(sizeof(Icon));
            memcpy(iconp, &icon, sizeof(Icon));

            ConnectedViewModel_t* vm = view_get_model(app->connected_view);
            IconDict_set_at(vm->icons, icon_add_data->icon_id, iconp);
            view_commit_model(app->connected_view, false);

            break;
        }
        case HW_SPEAKER_PLAY_ID: {
            SpeakerPlayData_t* speaker_play_data = proto_data->data;
            if (!furi_hal_speaker_is_mine() && !furi_hal_speaker_acquire(25)) {
                FURI_LOG_E(LOG_TAG, "Failed to acquire speaker");
                break;
            }
            furi_hal_speaker_start(speaker_play_data->frequency, speaker_play_data->volume);
            break;
        }
        case HW_SPEAKER_STOP_ID: {
            if (!furi_hal_speaker_is_mine()) {
                break;
            }
            furi_hal_speaker_stop();
            furi_hal_speaker_release();
            break;
        }
        case HW_VIBRATOR_ON_ID: {
            furi_hal_vibro_on(true);
            break;
        }
        case HW_VIBRATOR_OFF_ID: {
            furi_hal_vibro_on(false);
            break;
        }
        case HW_LIGHT_SET_ID: {
            LightSetData_t* light_set_data = proto_data->data;
            furi_hal_light_set(light_set_data->light, light_set_data->value);
            break;
        }
        // Disabled for now as it cause code blocking with delay
        // case HW_LIGHT_SEQUENCE_ID: {
        //     LightSequenceData_t* light_sequence_data = proto_data->data;
        //     furi_hal_light_sequence(light_sequence_data->sequence);
        //     break;
        // }
        default: {
            FURI_LOG_E(LOG_TAG, "Unknown protocol id: %d", proto_data->id);
            break;
        }
    }
}

void cli_callback(Cli* cli, FuriString* args, void* ctx) {
    UNUSED(args);
    FURI_LOG_I(LOG_TAG, "CLI connection...");

    PPApp_t* app = ctx;
    if (app->cli_running) {
        return;
    }

    app->cli = cli;
    app->cli_running = true;

    view_dispatcher_switch_to_view(app->view_dispatcher, CONNECTED_VIEW_ID);    

    ConnectedViewModel_t* vm;

    vm = view_get_model(app->connected_view);
    IconDict_init(vm->icons);
    view_commit_model(app->connected_view, true);

    // connect
    protocol_send_empty(app->cli, CNT_FLIPPER_START_ID);

    while (app->cli_running) {
        if (!cli_is_connected(app->cli)) {
            app->cli_running = false;
            break;
        }

        // recieve payload
        ProtocolData_t* proto_data = protocol_receive(app->cli, 5);
        if (proto_data == NULL) {
            continue;
        }
        run_data(app, proto_data);
        protocol_data_free(proto_data);
    }

    // disconnect
    protocol_send_empty(app->cli, CNT_FLIPPER_STOP_ID);

    // cleanup
    app->cli = NULL;
    
    vm = view_get_model(app->connected_view);
    IconDict_clear(vm->icons);
    if (vm->draw_data != NULL) {
        for (uint16_t i = 0; i < vm->draw_data->draw_arr_size; i++) {
            protocol_data_free(&vm->draw_data->draw_arr[i]);
        }
        free(vm->draw_data->draw_arr);
        free(vm->draw_data);
        vm->draw_data = NULL;
    }
    view_commit_model(app->connected_view, true);

    // go back
    view_dispatcher_switch_to_view(app->view_dispatcher, DISCONNECTED_VIEW_ID);
}

// ##############
// STATE
PPApp_t* state_init() {
    FURI_LOG_I(LOG_TAG, "Initing app...");
    PPApp_t* app = malloc(sizeof(PPApp_t));

    app->view_dispatcher    = view_dispatcher_alloc();
    app->gui                = furi_record_open(RECORD_GUI);
    app->mutex              = furi_mutex_alloc(FuriMutexTypeNormal);
    app->cli                = NULL;

    app->cli_running = false;

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_enable_queue(app->view_dispatcher);

    app->connected_view = connected_view_alloc(app);
    app->disconnected_view = disconnected_view_alloc(app);

    return app;
}

void state_free(PPApp_t* app) {
    FURI_LOG_I(LOG_TAG, "Freeing app...");
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    view_free(app->connected_view);
    view_free(app->disconnected_view);
    furi_mutex_free(app->mutex);
    free(app);
}

// ##############
// ENTRY
int32_t python_playground_app(void) {
    PPApp_t* app = state_init();
    
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "python_playground", CliCommandFlagParallelSafe, cli_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, DISCONNECTED_VIEW_ID);
    view_dispatcher_run(app->view_dispatcher);

    cli_delete_command(cli, "python_playground");
    furi_record_close(RECORD_CLI);

    state_free(app);

    return 0;
}
