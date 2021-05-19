#include "helper.h"

#include "pamac.h"
#include <gio/gio.h>
#include <gmodule.h>

#include <QEventLoop>
#include "../root-actions.h"


ActionReply MyHelper::changetheme( const QVariantMap &args ){
    QString theme = args["theme"].toString();

    int res;

    if( theme == "boot log" )
        res = rootActions::setLog();
    else if( theme == "black screen" )
        res = rootActions::disable();
    else{
        res = rootActions::setTheme( theme );
    }

    return res? ActionReply::SuccessReply() :
                ActionReply::HelperErrorReply();
}


ActionReply MyHelper::install( const QVariantMap &args ){

    QStringList toInstall = args["toInstall"].toStringList();

    PamacConfig *conf = pamac_config_new( "/etc/pamac.conf" );
    PamacDatabase *database = pamac_database_new( conf );
    PamacTransaction *transaction = pamac_transaction_new( database );

    for( const QString& s : qAsConst(toInstall) ){

        if( (int)pamac_database_search_pkgs(database, s.toUtf8())->len == 0 )
            pamac_transaction_add_aur_pkg_to_build( transaction, s.toUtf8() );
        else
            pamac_transaction_add_pkg_to_install(   transaction, s.toUtf8() );
    }

    QEventLoop l(this);
    pamac_transaction_run_async( transaction,
                                 (GAsyncReadyCallback)+[]( PamacDatabase* obj,
                                                           bool success,
                                                           QEventLoop* t )
                                 {
                                     Q_UNUSED(obj);
                                     t->quit();
                                 },
                                 &l   );


    l.exec();
    pamac_transaction_quit_daemon( transaction );


    return ActionReply::SuccessReply();
}

ActionReply MyHelper::remove( const QVariantMap &args ){

    QString theme = args["theme"].toString();

    PamacConfig *conf = pamac_config_new( "/etc/pamac.conf" );
    PamacDatabase *database = pamac_database_new( conf );
    PamacTransaction *transaction = pamac_transaction_new( database );

    pamac_transaction_add_pkg_to_remove( transaction, theme.toUtf8() );

    QEventLoop l(this);
    pamac_transaction_run_async( transaction,
                                 (GAsyncReadyCallback)+[]( PamacDatabase* obj,
                                                           bool success,
                                                           QEventLoop* t )
                                 {
                                     Q_UNUSED(obj);
                                     t->quit();
                                 },
                                 &l   );

    l.exec();
    pamac_transaction_quit_daemon( transaction );

    return ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("dev.android7890.bootsplashmanager", MyHelper)
