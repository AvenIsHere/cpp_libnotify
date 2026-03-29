#ifndef CPP_LIBNOTIFY_LIBRARY_H
#define CPP_LIBNOTIFY_LIBRARY_H

#include <functional>
#include <string>
#include <memory>

extern "C" {
    typedef struct _NotifyNotification NotifyNotification; // NOLINT(bugprone-reserved-identifier)
    typedef struct _GVariant GVariant; // NOLINT(bugprone-reserved-identifier)
}

enum class Urgency {
    LOW,
    NORMAL,
    CRITICAL,
};

class Notification {
    struct Notification_Deleter { void operator()(NotifyNotification* n) const; };
    struct Variant_Deleter { void operator()(GVariant* v) const; };

    using notify_notification = std::unique_ptr<NotifyNotification, Notification_Deleter>;
    using g_variant = std::unique_ptr<GVariant, Variant_Deleter>;

    notify_notification m_notification;
public:
    // constructors
    explicit Notification(const std::string& given_summary,
                          const std::string& given_body,
                          const std::string& given_icon);
    explicit Notification(NotifyNotification* given_notification);

    // setters (they return references themselves to make chaining possible)
    Notification& set_notification(notify_notification given_notification);
    Notification& set_summary(const std::string& given_summary);
    Notification& set_body(const std::string& given_body);
    Notification& set_icon(const std::string& given_icon);
    Notification& set_urgency(Urgency given_urgency);
    Notification& set_timeout(int given_timeout);
    Notification& set_hint(const std::string& given_key);

    using ActionCallback = std::function<void(Notification&, const std::string&)>;
    Notification& add_action(const std::string& given_id,
                             const std::string& given_label,
                             ActionCallback given_callback);

    // getters
    [[nodiscard]] const NotifyNotification* get_NotifyNotification() const;
    [[nodiscard]] std::string get_summary() const;
    [[nodiscard]] std::string get_body() const;
    [[nodiscard]] std::string get_icon() const;

    // static functions
    static bool init(const std::string& name);
    static bool de_init();

    // member functions
    bool show();
};

#endif // CPP_LIBNOTIFY_LIBRARY_H