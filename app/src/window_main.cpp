#include "window_main.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "settings_window.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <stdexcept>

namespace otterboard::app {

// ─────────────────────────────────────────────────────────────────────────────
// Константы
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int COL_WIDTH = 280;
static constexpr int TASK_CARD_HEIGHT = 72;
static constexpr int DAY_BUTTON_SIZE = 32;

// ─────────────────────────────────────────────────────────────────────────────
// Конструктор / деструктор
// ─────────────────────────────────────────────────────────────────────────────
window_main::window_main(const window_main::setup &setup,
                         const window_main::state &state)
    : setup_(setup), state_(state) {
  // Инициализируем настройки из setup
  state_.main_state.app_settings.storage_type = setup_.storage_type;
  state_.main_state.app_settings.storage_path = setup_.storage_path;

  // Создаём хранилище
  apply_settings();

  // Загружаем данные
  bool loaded = false;
  if (storage_) {
    storage_->load_board(state_.main_state.board);
    storage_->load_events(state_.main_state.calendar_events);
    storage_->load_notes(state_.main_state.notes);
    loaded = true;
  }

  // Если доска пустая — создаём стандартные колонки
  if (state_.main_state.board.columns.empty()) {
    init_default_board();
  }

  main_thread_ = std::thread(std::bind(&window_main::run, this));
}

window_main::~window_main() {
  if (main_thread_.joinable()) {
    main_thread_.join();
  }
}

int window_main::get_exit_code() {
  if (main_thread_.joinable()) {
    main_thread_.join();
  }
  return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Вспомогательные функции
// ─────────────────────────────────────────────────────────────────────────────

void window_main::init_default_board() {
  auto &cols = state_.main_state.board.columns;
  cols.clear();
  for (const char *name : {"To Do", "In Progress", "Done"}) {
    common::Column col;
    col.title = name;
    cols.push_back(std::move(col));
  }
}

void window_main::save_all() {
  if (!storage_)
    return;
  if (!state_.main_state.app_settings.auto_save)
    return;
  storage_->save_board(state_.main_state.board);
  storage_->save_events(state_.main_state.calendar_events);
  storage_->save_notes(state_.main_state.notes);
}

void window_main::apply_settings() {
  const auto &s = state_.main_state.app_settings;
  storage::backend_type bt = (s.storage_type == common::StorageType::Sqlite)
                                 ? storage::backend_type::sqlite
                                 : storage::backend_type::json_file;
  storage_ = storage::open_kanban_storage(bt, s.storage_path);
}

common::Task *window_main::find_task(int64_t id, int &out_col_idx) {
  for (int i = 0; i < (int)state_.main_state.board.columns.size(); ++i) {
    auto &tasks = state_.main_state.board.columns[i].tasks;
    for (auto &t : tasks) {
      if (t.id == id) {
        out_col_idx = i;
        return &t;
      }
    }
  }
  out_col_idx = -1;
  return nullptr;
}

common::CalendarEvent *window_main::find_event(int64_t id) {
  for (auto &e : state_.main_state.calendar_events) {
    if (e.id == id)
      return &e;
  }
  return nullptr;
}

void window_main::open_task_editor(int64_t task_id) {
  state_.editing_task_id = task_id;
  state_.open_task_popup = true;

  if (task_id >= 0) {
    int col_idx = -1;
    auto *t = find_task(task_id, col_idx);
    if (t) {
      strncpy(state_.task_title_buf, t->title.c_str(), 255);
      strncpy(state_.task_desc_buf, t->description.c_str(), 1023);
      strncpy(state_.task_date_buf, t->date_due.c_str(), 31);
      state_.task_priority = (int)t->priority;
      memcpy(state_.task_color, t->color.data(), sizeof(float) * 4);
      state_.task_tags_buf[0] = '\0';
      for (size_t i = 0; i < t->tags.size() && i < 250; ++i) {
        if (i > 0)
          strncat(state_.task_tags_buf, ",", 255);
        strncat(state_.task_tags_buf, t->tags[i].c_str(), 255);
      }
    }
  }
}

void window_main::open_event_editor(int64_t event_id) {
  state_.editing_event_id = event_id;
  state_.open_event_popup = true;

  if (event_id >= 0) {
    auto *e = find_event(event_id);
    if (e) {
      strncpy(state_.event_title_buf, e->title.c_str(), 255);
      strncpy(state_.event_desc_buf, e->description.c_str(), 1023);
    }
  }
}

void window_main::load_note_into_editor(int note_idx) {
  if (note_idx < 0 || note_idx >= (int)state_.main_state.notes.size()) {
    state_.selected_note_idx = -1;
    state_.note_title_buf[0] = '\0';
    state_.note_content_buf[0] = '\0';
    return;
  }
  state_.selected_note_idx = note_idx;
  auto &n = state_.main_state.notes[note_idx];
  strncpy(state_.note_title_buf, n.title.c_str(), 255);
  strncpy(state_.note_content_buf, n.content.c_str(), 4095);
}

// ─────────────────────────────────────────────────────────────────────────────
// Main loop
// ─────────────────────────────────────────────────────────────────────────────
void window_main::run() {
  const char *glsl_version = "#version 330";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  int win_w = (int)state_.main_state.app_settings.window_width;
  int win_h = (int)state_.main_state.app_settings.window_height;

  window_ = glfwCreateWindow(win_w, win_h, "OtterBoard", nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    throw std::runtime_error("Failed to create a new OpenGL context");
  }

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsDark();

  // Font with Cyrillic support
  float base_font_size = 14.0f;
  ImFontConfig cfg;
  cfg.OversampleH = 1;
  cfg.OversampleV = 1;
  cfg.PixelSnapH = true;
  static const ImWchar ranges[] = {
      0x0020, 0x00FF, // Basic Latin + Latin Supplement
      0x0400, 0x044F, // Cyrillic
      0x2000, 0x206F, // General Punctuation
      0};
  cfg.GlyphRanges = ranges;
  const char *font_paths[] = {
      "/usr/share/fonts/truetype/lato/Lato-Regular.ttf",
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/TTF/dejavu-sans.ttf",
  };
  ImFont *font = nullptr;
  for (const char *fp : font_paths) {
    font = io.Fonts->AddFontFromFileTTF(fp, base_font_size, &cfg);
    if (font)
      break;
  }
  if (!font) {
    font = io.Fonts->AddFontDefault(&cfg);
  }

  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  while (!glfwWindowShouldClose(window_)) {
    lifecycle();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window_);
  glfwTerminate();
}

// ─────────────────────────────────────────────────────────────────────────────
// Перетасовка / календарь / заметки
// ─────────────────────────────────────────────────────────────────────────────
void window_main::lifecycle() {
  glfwPollEvents();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // ── Настройки ────────────────────────────────────────────────────────────
  if (state_.show_settings) {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize({420, 440}, ImGuiCond_Always);

    if (ImGui::Begin("Настройки", &state_.show_settings,
                     ImGuiWindowFlags_NoCollapse)) {
      bool changed = render_settings_window(state_.main_state.app_settings,
                                            state_.show_settings);
      if (changed) {
        apply_settings();
      }
    }
    ImGui::End();
  }

  // ── Основное окно ────────────────────────────────────────────────────────
  ImGuiIO &io = ImGui::GetIO();
  ImGui::SetNextWindowPos({0.0f, 0.0f});
  ImGui::SetNextWindowSize(io.DisplaySize);

  ImGui::Begin("##OtterBoard", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

  render_menu_bar();

  // ── Разделение экрана ────────────────────────────────────────────────────
  float win_w = ImGui::GetContentRegionAvail().x;
  float win_h = ImGui::GetContentRegionAvail().y;

  float kanban_w = win_w * 2.0f / 3.0f;
  float right_w = win_w - kanban_w - 8.0f; // 8 = gap

  float cal_h = win_h * 0.55f;
  float notes_h = win_h - cal_h - 4.0f;

  // ── Kanban-доска (левая 2/3) ─────────────────────────────────────────────
  ImGui::SetCursorPosX(4.0f);
  ImGui::BeginChild("##kanban", ImVec2(kanban_w, win_h),
                    ImGuiChildFlags_Borders,
                    ImGuiWindowFlags_HorizontalScrollbar);

  render_kanban_board();

  ImGui::EndChild();

  ImGui::SameLine();

  // ── Календарь (правая верхняя часть) ─────────────────────────────────────
  ImGui::SetCursorPosX(kanban_w + 12.0f);
  ImGui::BeginChild("##calendar", ImVec2(right_w, cal_h),
                    ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
  render_calendar();
  ImGui::EndChild();

  // ── Заметки (правая нижняя часть) ────────────────────────────────────────
  ImGui::SetCursorPosX(kanban_w + 12.0f);
  ImGui::BeginChild("##notes", ImVec2(right_w, notes_h),
                    ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
  render_notes();
  ImGui::EndChild();

  ImGui::End();

  // ── Попапы (модальные окна поверх всего) ─────────────────────────────────
  render_task_popup();
  render_column_popup();
  render_event_popup();

  // ── Рендер ───────────────────────────────────────────────────────────────
  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(window_, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);

  const auto &bg = state_.main_state.app_settings.background_color;
  glClearColor(bg[0], bg[1], bg[2], bg[3]);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window_);
}

// ─────────────────────────────────────────────────────────────────────────────
// Меню
// ─────────────────────────────────────────────────────────────────────────────
void window_main::render_menu_bar() {
  if (!ImGui::BeginMenuBar())
    return;

  if (ImGui::BeginMenu("Файл")) {
    if (ImGui::MenuItem("Сохранить", "Ctrl+S")) {
      save_all();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Выход", "Alt+F4")) {
      glfwSetWindowShouldClose(window_, true);
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Вид")) {
    if (ImGui::MenuItem("Настройки", "Ctrl+,", &state_.show_settings)) {
      // toggle
    }
    ImGui::EndMenu();
  }

  ImGui::EndMenuBar();
}

// ─────────────────────────────────────────────────────────────────────────────
// Kanban-доска
// ─────────────────────────────────────────────────────────────────────────────
void window_main::render_kanban_board() {
  auto &board = state_.main_state.board;

  // Сбрасываем подсветку позиции вставки — её установит drop-зона в этом кадре
  if (!ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
    state_.drop_hint_col = -1;
    state_.drop_hint_pos = -1;
  }

  // Кнопка "Добавить колонку"
  if (ImGui::Button("+ Добавить колонку", ImVec2(160, 28))) {
    common::Column col;
    col.id = common::next_id();
    board.columns.push_back(std::move(col));
  }

  // ── Рендер колонок ──────────────────────────────────────────────────────
  int nc = (int)board.columns.size();
  for (int i = 0; i < nc; ++i) {
    if (i > 0)
      ImGui::SameLine();
    render_column(i);
  }
}

void window_main::render_column(int col_idx) {
  auto &col = state_.main_state.board.columns[col_idx];

  // Цвет рамки заголовка по приоритету первой задачи (или нейтральный)
  ImVec4 header_col = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
  if (!col.tasks.empty()) {
    auto max_p =
        std::max_element(col.tasks.begin(), col.tasks.end(),
                         [](const common::Task &a, const common::Task &b) {
                           return a.priority < b.priority;
                         });
    if (max_p->priority == common::TaskPriority::High)
      header_col = ImVec4(0.85f, 0.25f, 0.25f, 1.0f);
    else if (max_p->priority == common::TaskPriority::Medium)
      header_col = ImVec4(0.85f, 0.65f, 0.0f, 1.0f);
    else
      header_col = ImVec4(0.25f, 0.65f, 0.25f, 1.0f);
  }

  // Заголовок колонки
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_Border, header_col);
  ImGui::BeginChild(std::string("##col_" + std::to_string(col.id)).c_str(),
                    ImVec2(COL_WIDTH, -1), ImGuiChildFlags_Borders,
                    ImGuiWindowFlags_None);

  // Заголовок с кнопками
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::BeginChild(std::string("##hdr_" + std::to_string(col.id)).c_str(),
                    ImVec2(-1, 36), ImGuiChildFlags_None,
                    ImGuiWindowFlags_NoBackground);
  ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "%s",
                     col.title.c_str());
  ImGui::SameLine();
  ImGui::Spacing();
  ImGui::Dummy(ImVec2(0, 0));
  ImGui::SameLine();
  ImGui::Spacing();
  ImGui::Dummy(ImVec2(0, 0));
  ImGui::SameLine();
  {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::Text("(%d)", (int)col.tasks.size());
    ImGui::PopStyleColor();
  }
  ImGui::SameLine();
  ImGui::Dummy(ImVec2(0, 0));
  ImGui::SameLine();
  {
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    if (ImGui::SmallButton("...")) {
      state_.editing_col_id = col.id;
      strncpy(state_.col_rename_buf, col.title.c_str(), 255);
      state_.open_col_popup = true;
    }
    ImGui::PopStyleColor(2);
  }
  ImGui::EndChild();
  ImGui::PopStyleColor(); // закрываем ChildBg заголовка (push перед BeginChild)

  // Контекстное меню колонки открывается модальным окном (render_column_popup)

  // ── Тело колонки: задачи ────────────────────────────────────────────────
  // Универсальный обработчик drop-зоны: вставка перед задачей index
  // (index == tasks.size() — в конец колонки).
  auto drop_zone = [&](int index) {
    if (!ImGui::BeginDragDropTarget())
      return;
    if (const ImGuiPayload *p = ImGui::AcceptDragDropPayload(
            "KANBAN_TASK", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
      state_.drop_hint_col = col_idx;
      state_.drop_hint_pos = index;

      if (p->IsDelivery()) {
        int64_t task_id = *(int64_t *)p->Data;
        int src_col = -1;
        auto *task = find_task(task_id, src_col);
        if (task) {
          // Копируем задачу ДО удаления — erase инвалидирует указатель
          common::Task moved = *task;
          if (src_col >= 0) {
            auto &src_tasks = state_.main_state.board.columns[src_col].tasks;
            src_tasks.erase(std::remove_if(src_tasks.begin(), src_tasks.end(),
                                           [&task_id](const common::Task &t) {
                                             return t.id == task_id;
                                           }),
                            src_tasks.end());
            // Индекс вставки мог сместиться после удаления из той же колонки
            if (src_col == col_idx && index > (int)col.tasks.size())
              index = (int)col.tasks.size();
          }
          auto &dest_tasks = state_.main_state.board.columns[col_idx].tasks;
          int insert_pos = std::max(0, std::min(index, (int)dest_tasks.size()));
          dest_tasks.insert(dest_tasks.begin() + insert_pos, std::move(moved));
          for (int k = 0; k < (int)dest_tasks.size(); ++k)
            dest_tasks[k].position = k;
          save_all();
        }
      }
    }
    ImGui::EndDragDropTarget();
  };

  // Рендер задач с drop-зонами между ними
  for (int j = 0; j < (int)col.tasks.size(); ++j) {
    drop_zone(j);
    render_task_card(col.tasks[j], col_idx);
  }

  // Drop-зона в конце колонки
  if (state_.drop_hint_col == col_idx &&
      state_.drop_hint_pos == (int)col.tasks.size()) {
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    dl->AddLine(p0, ImVec2(p0.x + COL_WIDTH - 12, p0.y),
                IM_COL32(80, 160, 255, 255), 2.0f);
  }
  drop_zone((int)col.tasks.size());

  // ── Кнопка "Добавить задачу" ────────────────────────────────────────────
  ImGui::Spacing();
  if (ImGui::SmallButton("+ Добавить задачу")) {
    common::Task t;
    t.id = common::next_id();
    t.position = (int)col.tasks.size();
    col.tasks.push_back(t);
    open_task_editor(t.id);
  }

  ImGui::EndChild(); // тело колонки
  ImGui::PopStyleColor(2);
}

void window_main::render_task_card(common::Task &task, int col_idx) {
  auto &io = ImGui::GetIO();
  float card_h = TASK_CARD_HEIGHT;
  float card_w = COL_WIDTH - 12;

  ImGui::PushID(task.id);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {4, 4});

  // Невидимая кнопка для drag-n-drop и кликов
  ImGui::InvisibleButton("##card", ImVec2(card_w, card_h));
  bool is_hovered = ImGui::IsItemHovered();
  bool is_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
  bool is_double_click =
      is_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

  // Рисуем рамку и фон
  ImDrawList *dl = ImGui::GetWindowDrawList();
  ImVec2 pos = ImGui::GetItemRectMin();
  ImVec4 border_color = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
  ImVec4 bg_color =
      ImVec4(task.color[0], task.color[1], task.color[2], task.color[3]);

  // Цвет рамки по приоритету
  if (task.priority == common::TaskPriority::High) {
    border_color = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
  } else if (task.priority == common::TaskPriority::Medium) {
    border_color = ImVec4(0.9f, 0.7f, 0.1f, 1.0f);
  }

  if (is_hovered) {
    bg_color = ImVec4(bg_color.x * 1.2f, bg_color.y * 1.2f, bg_color.z * 1.2f,
                      bg_color.w);
    border_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
  }

  ImVec2 pos_end(pos.x + card_w, pos.y + card_h);
  dl->AddRectFilled(pos, pos_end, ImGui::ColorConvertFloat4ToU32(bg_color),
                    4.0f);
  dl->AddRect(pos, pos_end, ImGui::ColorConvertFloat4ToU32(border_color), 4.0f,
              ImDrawFlags_None, 1.5f);

  // Заголовок
  float text_x = pos.x + 8;
  float text_y = pos.y + 8;
  dl->AddText(ImVec2(text_x, text_y), IM_COL32_WHITE,
              task.title.empty() ? "(без названия)" : task.title.c_str());

  // Дата
  if (!task.date_due.empty()) {
    std::string date_str = "\ue8a9 " + task.date_due; // unicode calendar
    dl->AddText(ImVec2(text_x, text_y + 16), IM_COL32(180, 180, 180, 255),
                task.date_due.c_str());
  }

  // Приоритет — маленькая метка
  const char *pri_label = "";
  ImVec4 pri_col = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  if (task.priority == common::TaskPriority::High) {
    pri_label = "ВЫСОКИЙ";
    pri_col = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
  } else if (task.priority == common::TaskPriority::Medium) {
    pri_label = "СРЕДНИЙ";
    pri_col = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
  }
  if (pri_label[0]) {
    float label_w = ImGui::CalcTextSize(pri_label).x + 8;
    float label_y = text_y + card_h - 20;
    dl->AddRectFilled({text_x, label_y}, {text_x + label_w, label_y + 14},
                      ImGui::ColorConvertFloat4ToU32(pri_col), 3.0f);
    dl->AddText(ImVec2(text_x + 4, label_y + 3), IM_COL32_BLACK, pri_label);
  }

  // Теги
  if (!task.tags.empty()) {
    float tag_x = text_x;
    float tag_y = text_y + 28;
    for (const auto &tag : task.tags) {
      float tw = ImGui::CalcTextSize(tag.c_str()).x + 8;
      dl->AddRectFilled(
          {tag_x, tag_y}, {tag_x + tw, tag_y + 14},
          ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.4f, 0.6f, 0.8f)), 2.0f);
      dl->AddText(ImVec2(tag_x + 4, tag_y + 3), IM_COL32_WHITE, tag.c_str());
      tag_x += tw + 4;
    }
  }

  // Кнопка удаления (появляется при наведении)
  if (is_hovered) {
    float del_x = pos.x + card_w - 20;
    float del_y = pos.y + 4;
    dl->AddRectFilled(
        {del_x, del_y}, {del_x + 16, del_y + 16},
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.8f, 0.2f, 0.2f, 0.7f)), 3.0f);
    dl->AddText(ImVec2(del_x + 3, del_y + 1), IM_COL32_WHITE, "x");

    if (is_clicked && ImGui::GetMousePos().x > del_x &&
        ImGui::GetMousePos().x < del_x + 16 && ImGui::GetMousePos().y > del_y &&
        ImGui::GetMousePos().y < del_y + 16) {
      auto &src_tasks = state_.main_state.board.columns[col_idx].tasks;
      int del_idx = -1;
      for (int k = 0; k < (int)src_tasks.size(); ++k) {
        if (src_tasks[k].id == task.id) {
          del_idx = k;
          break;
        }
      }
      if (del_idx >= 0)
        src_tasks.erase(src_tasks.begin() + del_idx);
      save_all();
      ImGui::PopStyleVar();
      ImGui::PopID();
      return;
    }
  }

  // Drag source — привязывается к InvisibleButton "##card"
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
    ImGui::SetDragDropPayload("KANBAN_TASK", &task.id, sizeof(int64_t));
    ImGui::Text("%s",
                task.title.empty() ? "(без названия)" : task.title.c_str());
    ImGui::EndDragDropSource();
  }

  // Double-click → open editor
  if (is_double_click) {
    open_task_editor(task.id);
  }

  ImGui::PopStyleVar();
  ImGui::PopID();

  ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
// Попап задачи
// ─────────────────────────────────────────────────────────────────────────────
void window_main::render_task_popup() {
  if (!state_.open_task_popup)
    return;

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize({420, 400}, ImGuiCond_Always);

  bool open = true;
  if (ImGui::Begin("Задача", &open,
                   ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse)) {
    ImGui::PushItemWidth(300.0f);

    ImGui::Text("Заголовок");
    ImGui::InputText("##task_title", state_.task_title_buf, 255);

    ImGui::Spacing();
    ImGui::Text("Описание");
    ImGui::InputTextMultiline("##task_desc", state_.task_desc_buf, 1024,
                              ImVec2(-1, 80));

    ImGui::Spacing();
    ImGui::Text("Дата (YYYY-MM-DD)");
    ImGui::InputText("##task_date", state_.task_date_buf, 31);

    ImGui::Spacing();
    ImGui::Text("Приоритет");
    static const char *pri_items[] = {"Низкий", "Средний", "Высокий"};
    ImGui::Combo("##task_priority", &state_.task_priority, pri_items,
                 IM_ARRAYSIZE(pri_items));

    ImGui::Spacing();
    ImGui::Text("Цвет карточки");
    ImGui::ColorEdit4("##task_color", state_.task_color);

    ImGui::Spacing();
    ImGui::Text("Теги (через запятую)");
    ImGui::InputText("##task_tags", state_.task_tags_buf, 255);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Сохранить", ImVec2(100, 0))) {
      // Сохраняем в state
      if (state_.editing_task_id >= 0) {
        int col_idx = -1;
        auto *task = find_task(state_.editing_task_id, col_idx);
        if (task) {
          task->title = state_.task_title_buf;
          task->description = state_.task_desc_buf;
          task->date_due = state_.task_date_buf;
          task->priority =
              static_cast<common::TaskPriority>(state_.task_priority);
          memcpy(task->color.data(), state_.task_color, sizeof(float) * 4);

          // Теги
          task->tags.clear();
          if (state_.task_tags_buf[0]) {
            char *buf = strdup(state_.task_tags_buf);
            char *token = std::strtok(buf, ",");
            while (token) {
              // trim
              while (*token == ' ')
                token++;
              char *end = token + strlen(token) - 1;
              while (end > token && *end == ' ')
                *end-- = '\0';
              if (token[0])
                task->tags.push_back(token);
              token = std::strtok(nullptr, ",");
            }
            free(buf);
          }
        }
      }
      save_all();
      state_.open_task_popup = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Удалить", ImVec2(100, 0))) {
      if (state_.editing_task_id >= 0) {
        int col_idx = -1;
        auto *task = find_task(state_.editing_task_id, col_idx);
        if (task && col_idx >= 0) {
          state_.main_state.board.columns[col_idx].tasks.erase(
              std::remove_if(
                  state_.main_state.board.columns[col_idx].tasks.begin(),
                  state_.main_state.board.columns[col_idx].tasks.end(),
                  [id = state_.editing_task_id](const common::Task &t) {
                    return t.id == id;
                  }),
              state_.main_state.board.columns[col_idx].tasks.end());
          save_all();
        }
      }
      state_.open_task_popup = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Отмена", ImVec2(100, 0))) {
      state_.open_task_popup = false;
    }
  }

  ImGui::End();
  state_.open_task_popup = open;
}

// ─────────────────────────────────────────────────────────────────────────────
// Попап переименования колонки
// ─────────────────────────────────────────────────────────────────────────────
void window_main::render_column_popup() {
  if (!state_.open_col_popup)
    return;

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize({300, 150}, ImGuiCond_Always);

  bool open = true;
  if (ImGui::Begin("Переименовать колонку", &open,
                   ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse)) {
    ImGui::InputText("##col_rename", state_.col_rename_buf, 255);

    if (ImGui::Button("Сохранить", ImVec2(-1, 0))) {
      for (auto &col : state_.main_state.board.columns) {
        if (col.id == state_.editing_col_id) {
          col.title = state_.col_rename_buf;
          break;
        }
      }
      save_all();
      state_.open_col_popup = false;
    }
  }
  ImGui::End();
  state_.open_col_popup = open;
}

// ─────────────────────────────────────────────────────────────────────────────
// Календарь
// ─────────────────────────────────────────────────────────────────────────────
void window_main::render_calendar() {
  auto &events = state_.main_state.calendar_events;

  // Навигация
  ImGui::Text("Календарь");
  ImGui::Separator();

  static const char *month_names[] = {
      "Январь", "Февраль", "Март",     "Апрель",  "Май",    "Июнь",
      "Июль",   "Август",  "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"};
  static const char *day_names[] = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};

  // Кнопки навигации
  if (ImGui::SmallButton("<")) {
    state_.cal_month--;
    if (state_.cal_month < 1) {
      state_.cal_month = 12;
      state_.cal_year--;
    }
  }
  ImGui::SameLine();
  ImGui::Text("%s %d", month_names[state_.cal_month - 1], state_.cal_year);
  ImGui::SameLine();
  if (ImGui::SmallButton(">")) {
    state_.cal_month++;
    if (state_.cal_month > 12) {
      state_.cal_month = 1;
      state_.cal_year++;
    }
  }

  ImGui::Spacing();

  // День недели
  float avail_w = ImGui::GetContentRegionAvail().x;
  float day_w = avail_w / 7.0f;

  for (int d = 0; d < 7; ++d) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::Button(day_names[d], ImVec2(day_w - 2, 18));
    ImGui::PopStyleColor();
    if (d < 6)
      ImGui::SameLine();
  }
  ImGui::Spacing();

  // Вычисляем день недели первого числа (0=Пн, 6=Вс)
  auto first_day_of_month = [this]() -> int {
    int y = state_.cal_year;
    int m = state_.cal_month;
    // Целлер — но проще: std::mktime
    // Используем упрощённую формулу Зellers (0=Вс, 1=Пн...)
    int q = 1;
    int mm = m;
    int yy = y;
    if (mm <= 2) {
      mm += 12;
      yy--;
    }
    int h = (q + (13 * (mm + 1)) / 5 + yy + yy / 4 - yy / 100 + yy / 400) % 7;
    // h: 0=Сб, 1=Вс, 2=Пн...6=Пт → переводим в 0=Пн..6=Вс
    int dow = (h + 5) % 7;
    return dow;
  };

  // Количество дней в месяце
  auto days_in_month = [this]() -> int {
    int m = state_.cal_month;
    int y = state_.cal_year;
    if (m == 2) {
      return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 29 : 28;
    }
    static const int dm[] = {0, 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return dm[m];
  };

  int fw = first_day_of_month();
  int dim = days_in_month();

  // Формируем строку даты для проверки событий
  auto date_str = [this](int d) -> std::string {
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", state_.cal_year,
             state_.cal_month, d);
    return std::string(buf);
  };

  // Проверка: есть ли событие на дату
  auto has_event = [&events](const std::string &ds) -> bool {
    for (const auto &e : events) {
      if (e.date == ds)
        return true;
    }
    return false;
  };

  // Сетка дней
  int day = 1;
  for (int row = 0; row < 6; ++row) {
    for (int col = 0; col < 7; ++col) {
      int cell = row * 7 + col;
      bool render_day = (cell >= fw && day <= dim);

      if (!render_day) {
        ImGui::Dummy(ImVec2(day_w - 2, DAY_BUTTON_SIZE));
      } else {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", day);

        bool has_ev = has_event(date_str(day));
        bool is_selected = (state_.cal_day == day);

        int pushed = 0;
        if (has_ev) {
          ImGui::PushStyleColor(ImGuiCol_Button,
                                ImVec4(0.2f, 0.4f, 0.7f, 0.6f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                ImVec4(0.3f, 0.5f, 0.9f, 0.8f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
          pushed += 3;
        }
        if (is_selected) {
          ImGui::PushStyleColor(ImGuiCol_Button,
                                ImVec4(0.15f, 0.3f, 0.6f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                ImVec4(0.25f, 0.5f, 1.0f, 1.0f));
          pushed += 3;
        }

        ImGui::PushStyleColor(ImGuiCol_Text,
                              has_ev ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
                                     : ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
        pushed += 1;

        ImVec2 btn_min = ImGui::GetCursorScreenPos();
        if (ImGui::Button(buf, ImVec2(day_w - 2, DAY_BUTTON_SIZE))) {
          state_.cal_day = day;
        }

        ImGui::PopStyleColor(pushed);

        // Маркер события — маленькая точка в углу кнопки
        if (has_ev) {
          ImDrawList *dl = ImGui::GetWindowDrawList();
          ImVec2 bb = ImGui::GetItemRectMax();
          dl->AddCircleFilled(ImVec2(bb.x - 4, bb.y - 4), 2.5f,
                              IM_COL32(255, 255, 255, 255));
        }

        ++day;
      }

      if (col < 6)
        ImGui::SameLine();
    }
  }

  ImGui::Spacing();
  ImGui::Separator();

  // Список событий выбранного дня
  std::string sel_date = date_str(state_.cal_day);
  ImGui::Text("События на %s", sel_date.c_str());

  auto day_events =
      [&events, &sel_date]() -> std::vector<const common::CalendarEvent *> {
    std::vector<const common::CalendarEvent *> result;
    for (const auto &e : events) {
      if (e.date == sel_date)
        result.push_back(&e);
    }
    return result;
  };

  auto d_events = day_events();
  if (d_events.empty()) {
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "(нет событий)");
  } else {
    for (const auto *ev : d_events) {
      ImGui::BulletText("%s", ev->title.empty() ? "(без названия)"
                                                : ev->title.c_str());
      ImGui::SameLine();
      if (ImGui::SmallButton(
              std::string("x" + std::to_string(ev->id)).c_str())) {
        events.erase(
            std::remove_if(events.begin(), events.end(),
                           [id = ev->id](const common::CalendarEvent &e) {
                             return e.id == id;
                           }),
            events.end());
        save_all();
      }
    }
  }

  if (ImGui::SmallButton("+ Добавить событие")) {
    state_.editing_event_id = -1;
    state_.open_event_popup = true;
    state_.event_title_buf[0] = '\0';
    state_.event_desc_buf[0] = '\0';
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Заметки
// ─────────────────────────────────────────────────────────────────────────────
void window_main::render_notes() {
  ImGui::Text("Заметки");
  ImGui::Separator();

  auto &notes = state_.main_state.notes;
  float avail_w = ImGui::GetContentRegionAvail().x;

  // Верхняя панель: список + кнопка
  ImGui::BeginChild("##note_list", ImVec2(avail_w, 80),
                    ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY);

  if (notes.empty()) {
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "(нет заметок)");
  } else {
    for (int i = 0; i < (int)notes.size(); ++i) {
      bool selected = (state_.selected_note_idx == i);
      char label[256];
      snprintf(label, sizeof(label), "##note_%d", i);
      if (ImGui::Selectable(label, selected,
                            ImGuiSelectableFlags_SpanAllColumns,
                            ImVec2(-1, 24))) {
        load_note_into_editor(i);
      }
      if (selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
  }

  if (ImGui::SmallButton("+ Новая заметка")) {
    common::Note n;
    n.id = common::next_id();
    notes.push_back(std::move(n));
    load_note_into_editor((int)notes.size() - 1);
  }

  ImGui::EndChild();

  ImGui::Spacing();

  // Редактор
  if (state_.selected_note_idx >= 0 &&
      state_.selected_note_idx < (int)notes.size()) {
    auto &note = notes[state_.selected_note_idx];

    ImGui::Text("Заголовок");
    ImGui::InputText("##note_title", state_.note_title_buf, 255);

    ImGui::Spacing();
    ImGui::Text("Текст");
    ImGui::InputTextMultiline("##note_content", state_.note_content_buf, 4096,
                              ImVec2(avail_w - 8, 120));

    ImGui::Spacing();
    if (ImGui::Button("Сохранить", ImVec2(100, 0))) {
      note.title = state_.note_title_buf;
      note.content = state_.note_content_buf;
      save_all();
    }
    ImGui::SameLine();
    if (ImGui::Button("Удалить", ImVec2(100, 0))) {
      notes.erase(notes.begin() + state_.selected_note_idx);
      state_.selected_note_idx = -1;
      state_.note_title_buf[0] = '\0';
      state_.note_content_buf[0] = '\0';
      save_all();
    }
    ImGui::SameLine();
    if (ImGui::Button("Отмена", ImVec2(100, 0))) {
      state_.selected_note_idx = -1;
      state_.note_title_buf[0] = '\0';
      state_.note_content_buf[0] = '\0';
    }
  } else {
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                       "Выберите заметку или создайте новую");
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Попап события
// ─────────────────────────────────────────────────────────────────────────────
void window_main::render_event_popup() {
  if (!state_.open_event_popup)
    return;

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize({400, 250}, ImGuiCond_Always);

  bool open = true;
  if (ImGui::Begin("Событие", &open,
                   ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse)) {
    ImGui::PushItemWidth(300.0f);

    ImGui::Text("Заголовок");
    ImGui::InputText("##event_title", state_.event_title_buf, 255);

    ImGui::Spacing();
    ImGui::Text("Описание");
    ImGui::InputTextMultiline("##event_desc", state_.event_desc_buf, 1024,
                              ImVec2(-1, 60));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Сохранить", ImVec2(100, 0))) {
      common::CalendarEvent ev;
      if (state_.editing_event_id >= 0) {
        ev.id = state_.editing_event_id;
        auto *existing = find_event(ev.id);
        if (existing) {
          existing->title = state_.event_title_buf;
          existing->description = state_.event_desc_buf;
          char dbuf[32];
          snprintf(dbuf, sizeof(dbuf), "%04d-%02d-%02d", state_.cal_year,
                   state_.cal_month, state_.cal_day);
          existing->date = dbuf;
          save_all();
        }
      } else {
        ev.id = common::next_id();
        ev.title = state_.event_title_buf;
        ev.description = state_.event_desc_buf;
        char dbuf[32];
        snprintf(dbuf, sizeof(dbuf), "%04d-%02d-%02d", state_.cal_year,
                 state_.cal_month, state_.cal_day);
        ev.date = dbuf;
        state_.main_state.calendar_events.push_back(std::move(ev));
        save_all();
      }
      state_.open_event_popup = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Отмена", ImVec2(100, 0))) {
      state_.open_event_popup = false;
    }
  }

  ImGui::End();
  state_.open_event_popup = open;
}

// ─────────────────────────────────────────────────────────────────────────────
// Геттеры
// ─────────────────────────────────────────────────────────────────────────────
const window_main::state &window_main::get_state() const { return state_; }

} // namespace otterboard::app
