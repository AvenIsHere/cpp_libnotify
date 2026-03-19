#ifndef CPP_LIBNOTIFY_LIBRARY_H
#define CPP_LIBNOTIFY_LIBRARY_H

#include <chrono>
#include <string>
#include <libnotify/notification.h>

struct NotificationDeleter {
    void operator()(NotifyNotification* n) {
        if (n) g_object_unref(n);
    }
};

class Notification {
    std::unique_ptr<NotifyNotification, NotificationDeleter> m_notification;
public:
    // constructors
    explicit Notification(const std::string& given_summary = "",
                          const std::string& given_body = "",
                          const std::string& given_icon = "");
    explicit Notification(NotifyNotification* given_notification);

    // setters (they return references themselves to make chaining possible)
    Notification& set_notification(NotifyNotification*& given_notification);
    Notification& set_summary(const std::string& given_summary);
    Notification& set_body(const std::string& given_body);

    // getters
    [[nodiscard]] NotifyNotification* get_NotifyNotification() const;
    [[nodiscard]] std::string get_summary() const;
    [[nodiscard]] std::string get_body() const;
    [[nodiscard]] std::string get_icon() const;

    // static functions
    static bool init(const std::string& name);
    static bool de_init();

    // member functions
    int send();
};

#endif // CPP_LIBNOTIFY_LIBRARY_H