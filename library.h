// Copyright (c) 2026 Aven Furness
// cpp-libnotify is available from https://github.com/AvenIsHere/cpp_libnotify
// Licensed under the MIT License.

#ifndef CPP_LIBNOTIFY_LIBRARY_H
#define CPP_LIBNOTIFY_LIBRARY_H

#include <functional>
#include <map>
#include <string>
#include <memory>
#include <mutex>

extern "C" {
typedef struct _NotifyNotification NotifyNotification; // NOLINT(bugprone-reserved-identifier)
}

class Notification {
public:
    enum class Urgency {
        LOW,
        NORMAL,
        CRITICAL,
    };
private:
    struct State;

    struct Notification_Deleter {
        void operator()(NotifyNotification *n) const;
    };

    using notify_notification = std::unique_ptr<NotifyNotification, Notification_Deleter>;
    using ActionCallback = std::function<void(std::shared_ptr<State>, const std::string &)>;

    struct State {
        notify_notification m_notification;
        std::map<std::string, ActionCallback> m_callbacks;

        explicit State(NotifyNotification *n) : m_notification(n) {
        }
    };

    std::shared_ptr<State> state;
    static std::string app_name;
    static std::once_flag initialised;

    static void ensure_initialised();

public:
    // constructors
    explicit Notification(const std::string &given_summary,
                          const std::string &given_body,
                          const std::string &given_icon = "");

    explicit Notification(notify_notification given_notification);

    // setters (they return references themselves to make chaining possible)
    Notification &set_notification(notify_notification given_notification);

    Notification &set_summary(const std::string &given_summary);

    Notification &set_body(const std::string &given_body);

    Notification &set_icon(const std::string &given_icon);

    Notification &set_urgency(Urgency given_urgency);

    Notification &set_timeout(int given_timeout);

    Notification &set_hint(const std::string &given_key, int given_value);

    Notification &set_hint(const std::string &given_key, std::string &given_value);

    Notification &set_hint(const std::string &given_key, bool given_value);

    Notification &add_action(const std::string &given_id,
                             const std::string &given_label,
                             ActionCallback given_callback);

    // getters
    [[nodiscard]] const NotifyNotification *get() const;

    [[nodiscard]] std::string get_summary() const;

    [[nodiscard]] std::string get_body() const;

    [[nodiscard]] std::string get_icon() const;

    // static functions
    static void set_app_name(const std::string &name);

    static void action_callback_trampoline(NotifyNotification *n, const char *action, void *user_data);

    // member functions
    bool show() const;

    bool close() const;

    // prevents copying
    Notification(const Notification &) = delete;

    Notification &operator=(const Notification &) = delete;

    // allows moving
    Notification(Notification &&) noexcept = default;

    Notification &operator=(Notification &&) noexcept = default;

    // destructor
    ~Notification() = default;
};

#endif // CPP_LIBNOTIFY_LIBRARY_H
