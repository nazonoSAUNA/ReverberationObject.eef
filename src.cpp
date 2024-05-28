#include <windows.h>
#include <exedit.hpp>

inline static char name[] = "残響オブジェクト";

constexpr int track_n = 2;
static char* track_name[track_n] = { const_cast<char*>("前レイヤー"), const_cast<char*>("対象数") };
static int track_default[track_n] = { 0, 2 };
static int track_s[track_n] = { -99, 1 };
static int track_e[track_n] = { 0, 100 };
static int track_scale[track_n] = { 1, 1 };
static int track_drag_min[track_n] = { -10, 1 };
static int track_drag_max[track_n] = { 0, 10 };

constexpr int check_n = 0;

inline static struct {
    ExEdit::Object** ObjectArrayPointer = (ExEdit::Object**)0x1e0fa4;
    ExEdit::Object** SortedObjectPointerTable = (ExEdit::Object**)0x168fa8;
    int* ObjectCount = (int*)0x146250;
    // int* SortedObjectLayerIndexBegin = (int*)0x149670;
    // int* SortedObjectLayerIndexEnd = (int*)0x135ac8;
    ExEdit::Filter** LoadedFilterTable = (ExEdit::Filter**)0x187c98;
    void(__cdecl* StoreFilter)(ExEdit::Object* obj, int filter_idx, int* track_ptr, int* check_ptr, void* exdata_ptr, ExEdit::FilterProcInfo* efpip) = (decltype(StoreFilter))0x47e30;
    // void(__cdecl* RestoreFilter)(ExEdit::Object* obj, int filter_idx, int* track_ptr, int* check_ptr, void* exdata_ptr) = (decltype(RestoreFilter))0x48230;
}ee;


int get_exedit_dll_hinst(ExEdit::Filter* efp) {
    constexpr int exedit92_exfunc_address = 0xa41e0;
    return (int)efp->exfunc - exedit92_exfunc_address;
}

int get_object_idx(ExEdit::Object* obj) {
    return ((int)obj - (int)*ee.ObjectArrayPointer) / sizeof(*obj);
}
void get_object_table_index(int scene, int layer, int* start, int* end) {
    *start = -1; *end = -2;
    int i;
    for (i = 0; i < *ee.ObjectCount; i++) {
        auto obj = ee.SortedObjectPointerTable[i];
        if (obj->scene_set == scene) {
            break;
        }
    }
    for (; i < *ee.ObjectCount; i++) {
        auto obj = ee.SortedObjectPointerTable[i];
        if (obj->scene_set != scene) {
            return;
        }
        if (obj->layer_set == layer) {
            *start = i;
            *end = i;
            break;
        }
    }
    for (; i < *ee.ObjectCount; i++) {
        auto obj = ee.SortedObjectPointerTable[i];
        if (obj->scene_set != scene || obj->layer_set != layer) {
            break;
        }
        *end = i;
    }
}
void play_audio_reverberation(ExEdit::ObjectFilterIndex ofi, ExEdit::FilterProcInfo* efpip) {
    memset(efpip->audio_data, 0, efpip->audio_n * efpip->audio_ch * sizeof(short));
    auto obj = *ee.ObjectArrayPointer + LOWORD(ofi) - 1;
    int track[64];

    for (int filter_idx = HIWORD(ofi); filter_idx < 12; filter_idx++) {
        if (obj->filter_param[filter_idx].id < 0) break;
        auto efp = ee.LoadedFilterTable[obj->filter_param[filter_idx].id];
        ExEdit::Object::FilterStatus flag;
        if (obj->index_midpt_leader < 0) {
            flag = obj->filter_status[filter_idx];
        } else {
            flag = (*(*ee.ObjectArrayPointer + obj->index_midpt_leader)).filter_status[filter_idx];
        }
        if (!has_flag(flag, ExEdit::Object::FilterStatus::Active) && has_flag(efp->flag, ExEdit::Filter::Flag::Input)) break;
        if (has_flag(flag, ExEdit::Object::FilterStatus::Active) && !has_flag(efp->flag, ExEdit::Filter::Flag::Input)) {
            if (efp->func_proc != nullptr) {
                ee.StoreFilter(obj, filter_idx, track, nullptr, nullptr, efpip);
                if (!efp->func_proc(efp, efpip)) break;
            }
        }
    }
}

#define SEARCH_FLAG_OBJECT_CAMERA_TARGET 0x1
#define SEARCH_FLAG_FILTER_INPUT 0x8
#define SEARCH_FLAG_FILTER_OUTPUT 0x10
#define SEARCH_FLAG_FILTER_EFFECT 0x20
#define SEARCH_FLAG_FILTER_CAMERA_CONTROL 0x40
#define SEARCH_FLAG_OBJECT_NOT_AUDIO 0x10000
#define SEARCH_FLAG_OBJECT_AUDIO 0x20000

BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
    int layer = min(0, efp->track[0]) + efp->layer_set;
    if (layer < 0) return FALSE;
    int pre_frame = efp->frame_start_chain - 1;
    if (layer == efp->layer_set) {
        if (pre_frame < 0) return FALSE;
        auto ofi = efp->exfunc->x1c(pre_frame, layer, efp->scene_set, nullptr, SEARCH_FLAG_FILTER_INPUT | SEARCH_FLAG_OBJECT_AUDIO); // search_object
        if ((int)ofi != 0) {
            play_audio_reverberation(ofi, efpip);
        }
    } else {
        int count = max(1, efp->track[1]);
        int s, e;
        get_object_table_index(efp->scene_set, layer, &s, &e);
        for (int i = e; s <= i; i--) {
            auto obj = ee.SortedObjectPointerTable[i];
            if (obj->frame_end < pre_frame) break;
            if (obj->frame_end < efpip->frame_num + efpip->add_frame && has_flag(obj->flag, ExEdit::Object::Flag::Sound)) {
                auto ofi = efp->exfunc->x20((ExEdit::ObjectFilterIndex)(get_object_idx(obj) + 0x00100001), nullptr, SEARCH_FLAG_FILTER_INPUT); // 1番目のフィルタから対象にする場合は12<=ofi.filterにする必要がある
                if ((int)ofi != 0) {
                    play_audio_reverberation(ofi, efpip);
                    if (--count <= 0) {
                        break;
                    }
                }
            }
        }
    }

    return FALSE;
}
BOOL func_init(ExEdit::Filter* efp) {
    int exedit_dll_hinst = get_exedit_dll_hinst(efp);
    int* ptr = (int*)&ee;
    for (int i = sizeof(ee) >> 2; 0 < i; i--) {
        *ptr += exedit_dll_hinst;
        ptr++;
    }

    return TRUE;
}


ExEdit::Filter effect_ef = {
    .flag = ExEdit::Filter::Flag::Audio | ExEdit::Filter::Flag::Input,
    .name = name,
    .track_n = track_n,
    .track_name = track_name,
    .track_default = track_default,
    .track_s = track_s,
    .track_e = track_e,
    .check_n = check_n,
    .func_proc = &func_proc,
    .func_init = &func_init,
    .track_scale = track_scale,
    .track_drag_min = track_drag_min,
    .track_drag_max = track_drag_max,
};

ExEdit::Filter* filter_list[] = {
    &effect_ef,
    NULL
};
EXTERN_C __declspec(dllexport)ExEdit::Filter** __stdcall GetFilterTableList() {
    return filter_list;
}
