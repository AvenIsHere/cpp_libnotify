// Copyright (c) 2026 Aven Furness
// cpp-libnotify is available from https://github.com/AvenIsHere/cpp_libnotify
// Licensed under the MIT License.

#include "Notification.h"
#include <libnotify/notify.h>

std::string Notification::app_name = "Unnamed App";
std::once_flag Notification::initialised;

void Notification::Notification_Deleter::operator()(NotifyNotification *n) const {
    if (n) {
        g_object_unref(n);
    }
}

void Notification::ensure_initialised() {
    std::call_once(initialised, []() {
        if (!notify_is_initted()) {
            notify_init(app_name.c_str());
            std::atexit([]() {
                if (notify_is_initted()) {
                    notify_uninit();
                }
            });
        }
    });
}

Notification::Notification(const std::string &given_summary, const std::string &given_body,
    const std::string &given_icon) {
    ensure_initialised();
    NotifyNotification* notification = notify_notification_new(
            given_summary.c_str(),
            given_body.c_str(),
            given_icon.c_str());
    this->state = std::make_shared<State>(notification);
    this->state->summary = given_summary;
    this->state->body = given_body;
    this->state->icon = given_icon;
    this->state->m_callbacks = std::map<std::string, ActionCallback>();
}

Notification & Notification::set_summary(const std::string &given_summary) {
    notify_notification_update(
        this->state->m_notification.get(),
        given_summary.c_str(),
        this->get_body().c_str(),
        this->get_icon().c_str());
    this->state->summary = given_summary;
    return *this;
}

Notification & Notification::set_body(const std::string &given_body) {
    notify_notification_update(
        this->state->m_notification.get(),
        this->get_summary().c_str(),
        given_body.c_str(),
        this->get_icon().c_str());
    this->state->body = given_body;
    return *this;
}

Notification & Notification::set_icon(const std::string &given_icon) {
    notify_notification_update(
        this->state->m_notification.get(),
        this->get_summary().c_str(),
        this->get_body().c_str(),
        given_icon.c_str());
    this->state->icon = given_icon;
    return *this;
}

Notification & Notification::set_urgency(const Urgency given_urgency) {
    NotifyUrgency lib_urgency;
    switch (given_urgency) {
        case Urgency::CRITICAL: lib_urgency = NOTIFY_URGENCY_CRITICAL; break;
        case Urgency::LOW: lib_urgency = NOTIFY_URGENCY_LOW; break;
        case Urgency::NORMAL:
        default: lib_urgency = NOTIFY_URGENCY_NORMAL; break;
    }
    notify_notification_set_urgency(this->state->m_notification.get(), lib_urgency);
    return *this;
}

Notification & Notification::set_timeout(const int given_timeout) {
    notify_notification_set_timeout(this->state->m_notification.get(), given_timeout);
    return *this;
}

Notification & Notification::set_hint(const std::string &given_key, const int given_value) {
    GVariant* value_g_variant = g_variant_new_int32(given_value);
    notify_notification_set_hint(
        this->state->m_notification.get(),
        given_key.c_str(),
        value_g_variant);
    g_variant_unref(value_g_variant);
    return *this;
}

Notification & Notification::set_hint(const std::string &given_key, const std::string &given_value) {
    GVariant* value_g_variant = g_variant_new_string(given_value.c_str());
    notify_notification_set_hint(
        this->state->m_notification.get(),
        given_key.c_str(),
        value_g_variant);
    g_variant_unref(value_g_variant);
    return *this;
}

Notification & Notification::set_hint(const std::string &given_key, const bool given_value) {
    GVariant* value_g_variant = g_variant_new_boolean(given_value);
    notify_notification_set_hint(
        this->state->m_notification.get(),
        given_key.c_str(),
        value_g_variant);
    g_variant_unref(value_g_variant);
    return *this;
}

Notification & Notification::add_action(const std::string &given_id, const std::string &given_label,
    ActionCallback given_callback) {

    auto* persistent_state = new std::shared_ptr(this->state);

    notify_notification_add_action(
        this->state->m_notification.get(),
        given_id.c_str(),
        given_label.c_str(),
        NOTIFY_ACTION_CALLBACK(Notification::action_callback_trampoline),
        persistent_state,
        [](gpointer data) {
            delete static_cast<std::shared_ptr<State>*>(data);
        });
    this->state->m_callbacks.emplace(given_id, given_callback);
    return *this;
}

std::string Notification::get_summary() const {
    return this->state->summary;
}

std::string Notification::get_body() const {
    return this->state->body;
}

std::string Notification::get_icon() const {
    return this->state->icon;
}

void Notification::set_app_name(const std::string &name) {
    app_name = name;
}

void Notification::action_callback_trampoline(NotifyNotification *n, const char *action, void *user_data) {
    const auto* state_ptr = static_cast<std::shared_ptr<State>*>(user_data);
    std::shared_ptr<State> state_shared = *state_ptr;

    if (state_shared->m_callbacks.contains(action)) {
        state_shared->m_callbacks.at(action)(state_shared, action);
    }
}

void Notification::show() const {
    GError* error = nullptr;
    notify_notification_show(this->state->m_notification.get(), &error);
    if (error) {
        g_error_free(error);
        throw std::runtime_error("Notification could not show");
    }
}

void Notification::close() const {
    GError* error = nullptr;
    notify_notification_close(this->state->m_notification.get(), &error);
    if (error) {
        g_error_free(error);
        throw std::runtime_error("Notification could not close");
    }
}