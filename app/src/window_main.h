#pragma once
#include <memory>
#include <string>
#include <thread>

#include <common/common.hpp>
#include <storage/kanban_storage.h>
#include <storage/storage.hpp>

class GLFWwindow;

namespace otterboard::app {

class window_main {
public:
  struct setup {
    common::StorageType storage_type = common::StorageType::JsonFile;
    std::string storage_path = "otterboard.json";
  };

  struct state {
    // Основное состояние (зеркало того, что лежит в хранилище)
    common::MainState main_state;

    // Флаг окна настроек
    bool show_settings = false;

    // --- Состояние попапа редактирования задачи ---
    bool open_task_popup = false;
    int64_t editing_task_id = -1;
    char task_title_buf[256] = {};
    char task_desc_buf[1024] = {};
    char task_date_buf[32] = {};
    int task_priority = 1;
    float task_color[4] = {0.8f, 0.8f, 0.8f, 1.0f};
    char task_tags_buf[256] = {};

    // --- Состояние попапа колонки (переименование) ---
    bool open_col_popup = false;
    int64_t editing_col_id = -1;
    char col_rename_buf[256] = {};

    // --- Состояние drag-n-drop ---
    int64_t drag_task_id = -1;
    int drag_src_col_idx = -1;

    // Подсветка позиции вставки при перетаскивании
    int drop_hint_col = -1;
    int drop_hint_pos = -1;

    // Отложенное перемещение задачи (применяется после рендера колонок)
    int64_t pending_drop_task = -1;
    int pending_drop_col = -1;
    int pending_drop_pos = -1;

    // --- Состояние календаря ---
    int cal_year = 2026;
    int cal_month = 10; // 1-based
    int cal_day = 1;    // выбранный день
    bool open_event_popup = false;
    int64_t editing_event_id = -1;
    char event_title_buf[256] = {};
    char event_desc_buf[1024] = {};

    // --- Состояние заметок ---
    int selected_note_idx = -1;
    char note_title_buf[256] = {};
    char note_content_buf[4096] = {};
    bool note_dirty = false;
  };

  window_main(const setup &setup, const state &state);
  ~window_main();

  int get_exit_code();
  const state &get_state() const;

private:
  void run();
  void lifecycle();

  void init_default_board();
  void save_all();
  void apply_settings();

  void render_menu_bar();
  void render_kanban_board();
  void render_column(int col_idx);
  void render_task_card(common::Task &task, int col_idx);
  void render_calendar();
  void render_notes();
  void render_task_popup();
  void render_column_popup();
  void render_event_popup();

  common::Task *find_task(int64_t id, int &out_col_idx);
  common::CalendarEvent *find_event(int64_t id);
  void open_task_editor(int64_t task_id);
  void open_event_editor(int64_t event_id);
  void load_note_into_editor(int note_idx);

  const setup setup_;
  state state_;

  std::unique_ptr<storage::kanban_storage> storage_;

  std::thread main_thread_;
  GLFWwindow *window_ = nullptr;
};

} // namespace otterboard::app
