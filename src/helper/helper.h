#include <KAuth>

using namespace KAuth;

class MyHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    ActionReply changetheme( const QVariantMap &args );
    ActionReply remove( const QVariantMap &args );
    ActionReply install( const QVariantMap &args );
};
