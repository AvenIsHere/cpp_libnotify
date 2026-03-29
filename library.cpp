#include "library.h"
#include <libnotify/notify.h>

void Notification::Notification_Deleter::operator()(NotifyNotification *n) const {
    if (n) {
        g_object_unref(n);
    }
}

std::string get_process_name() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        std::string path(result, count);
        return path.substr(path.find_last_of('/') + 1);
    }
    return "";
}

void Notification::ensure_initialised() {
    if (!notify_is_initted()) {
        notify_init(get_process_name().c_str());
    }
}

Notification::Notification(const std::string &given_summary, const std::string &given_body,
    const std::string &given_icon) {
    ensure_initialised();
    this->state->m_notification.reset(
        notify_notification_new(
            given_summary.c_str(),
            given_body.c_str(),
            given_icon.c_str()));
}

Notification::Notification(notify_notification given_notification) {
}

Notification & Notification::set_notification(notify_notification given_notification) {
    this->state->m_notification = std::move(given_notification);
    return *this;
}

Notification & Notification::set_summary(const std::string &given_summary) {
    notify_notification_update(
        this->state->m_notification.get(),
        given_summary.c_str(),
        this->get_body().c_str(),
        this->get_icon().c_str());
    return *this;
}

Notification & Notification::set_body(const std::string &given_body) {
    notify_notification_update(
        this->state->m_notification.get(),
        this->get_summary().c_str(),
        given_body.c_str(),
        this->get_icon().c_str());
    return *this;
}

Notification & Notification::set_icon(const std::string &given_icon) {
    notify_notification_update(
        this->state->m_notification.get(),
        this->get_summary().c_str(),
        this->get_body().c_str(),
        given_icon.c_str());
    return *this;
}

std::map<Notification::Urgency, NotifyUrgency> urgency_map = {
    {Notification::Urgency::CRITICAL, NOTIFY_URGENCY_CRITICAL},
    {Notification::Urgency::LOW, NOTIFY_URGENCY_LOW},
    {Notification::Urgency::NORMAL, NOTIFY_URGENCY_NORMAL}
};

Notification & Notification::set_urgency(Urgency given_urgency) {
    notify_notification_set_urgency(this->state->m_notification.get(), urgency_map.at(given_urgency));
    return *this;
}

Notification & Notification::set_timeout(const int given_timeout) {
    notify_notification_set_timeout(this->state->m_notification.get(), given_timeout);
    return *this;
}

Notification & Notification::set_hint(const std::string &given_key, int given_value) {
    notify_notification_set_hint(
        this->state->m_notification.get(),
        given_key.c_str(),
        reinterpret_cast<GVariant *>(given_value));
    return *this;
}

Notification & Notification::set_hint(const std::string &given_key, std::string &given_value) {
    notify_notification_set_hint(
        this->state->m_notification.get(),
        given_key.c_str(),
        reinterpret_cast<GVariant *>(given_value.data()));
    return *this;
}

Notification & Notification::set_hint(const std::string &given_key, const bool given_value) {
    notify_notification_set_hint(
        this->state->m_notification.get(),
        given_key.c_str(),
        reinterpret_cast<GVariant *>(given_value));
    return *this;
}

Notification & Notification::add_action(const std::string &given_id, const std::string &given_label,
    ActionCallback given_callback) {
    notify_notification_add_action(
        this->state->m_notification.get(),
        given_id.c_str(),
        given_label.c_str(),
        NOTIFY_ACTION_CALLBACK(Notification::action_callback_trampoline),
        this->state.get(),
        nullptr);
    this->state->m_callbacks.emplace(given_id, given_callback);
    return *this;
}

const NotifyNotification * Notification::get() const {
    return this->state->m_notification.get();
}

std::string Notification::get_summary() const {
    std::string summary;
    char* summary_char = nullptr;
    g_object_get(this->state->m_notification.get(), "summary", &summary_char, NULL);
    if (summary_char != nullptr) {
        summary = summary_char;
    }
    g_object_unref(summary_char);
    return summary;
}

std::string Notification::get_body() const {
    std::string body;
    char* body_char = nullptr;
    g_object_get(this->state->m_notification.get(), "body", &body_char, NULL);
    if (body_char != nullptr) {
        body = body_char;
    }
    g_object_unref(body_char);
    return body;
}

std::string Notification::get_icon() const {
    std::string icon;
    char* icon_char = nullptr;
    g_object_get(this->state->m_notification.get(), "icon", &icon_char, NULL);
    if (icon_char != nullptr) {
        icon = icon_char;
    }
    g_object_unref(icon_char);
    return icon;
}

void Notification::set_app_name(const std::string &name) {
    app_name = name;
}

void Notification::action_callback_trampoline(NotifyNotification *n, const char *action, void *user_data) {
    const auto* state_ptr = static_cast<std::shared_ptr<State>*>(user_data);

    if (state_ptr->get()->m_callbacks.contains(action)) {
        state_ptr->get()->m_callbacks[action](*state_ptr, action);
    }
}

bool Notification::show() const {
    GError* error;
    notify_notification_show(this->state->m_notification.get(), &error);
    return error;
}

bool Notification::close() const {
    GError* error;
    notify_notification_close(this->state->m_notification.get(), &error);
    return error;
}
