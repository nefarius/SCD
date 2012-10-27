#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>
#include <sqlite3.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <signal.h>


config_t cfg;
sqlite3 *db_con;

void clean_shutdown()
{
    config_destroy(&cfg);
    sqlite3_close(db_con);
    closelog();
}

void sig_handler(int signo)
{
    switch(signo)
    {
    case SIGINT:
        syslog(LOG_NOTICE, "SIGINT received, shutting down...");
        clean_shutdown();
        exit(0);
        break;
    case SIGUSR1:
        break;
    }
}

int main()
{
    sqlite3_stmt *db_stmt;
    int retval = 0;
    const char *config_file_name = "/etc/scd.cfg";
    const char *db_file_name;
    long kick_time = 15;

    config_init(&cfg);
    setlogmask(LOG_UPTO(LOG_NOTICE));

    openlog("scd", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    syslog(LOG_NOTICE, "Process started by user %d", getuid());
    syslog(LOG_NOTICE, "Parent PID is %d", getppid());

    // catch process termination fpr clean shutdown
    if(signal(SIGINT, sig_handler) == SIG_ERR)
    {
        syslog(LOG_WARNING, "Can not catch SIGINT, clean shutdown not possible");
    }
    else
    {
        syslog(LOG_INFO, "SIGINT will be handled");
    }

    // catch USR1 to reload configuration while running
    if(signal(SIGUSR1, sig_handler) == SIG_ERR)
    {
        syslog(LOG_WARNING, "Can not catch SIGUSR1, reloading configuration at runtime not possible");
    }
    else
    {
        syslog(LOG_INFO, "SIGUSR1 will be handled");
    }

    // check if config file is readyble
    if(access(config_file_name, R_OK) != -1)
    {
        // read in and check syntax of config file
        if(!config_read_file(&cfg, config_file_name))
        {
            syslog(LOG_ERR, "%s:%d - %s", config_file_name, config_error_line(&cfg), config_error_text(&cfg));
            clean_shutdown();
            return EXIT_FAILURE;
        }
    }
    else
    {
        syslog(LOG_ERR, "Could not open configuration file %s", config_file_name);
        clean_shutdown();
        return EXIT_FAILURE;
    }

    // get database location
    if(!config_lookup_string(&cfg, "database", &db_file_name))
    {
        syslog(LOG_ERR, "Could not read database location");
        clean_shutdown();
        return EXIT_FAILURE;
    }

    retval = sqlite3_open(db_file_name, &db_con);

    if(retval)
    {
        syslog(LOG_ERR, "sqlite3: %s", sqlite3_errmsg(db_con));
        clean_shutdown();
        return EXIT_FAILURE;
    }

    char *create_table = "CREATE TABLE IF NOT EXISTS scd_sessions "
                         "(ip TEXT PRIMARY KEY, user TEXT NOT NULL, "
                         "last_visit INTEGER NOT NULL)";

    retval = sqlite3_exec(db_con, create_table, 0, 0, 0);
    if(retval)
    {
        syslog(LOG_ERR, "sqlite3: %s", sqlite3_errmsg(db_con));
        clean_shutdown();
        return EXIT_FAILURE;
    }

    if(!config_lookup_int(&cfg, "kicktime", &kick_time))
    {
        syslog(LOG_NOTICE, "No global inactive session kick time set, using default value of %ld minutes", kick_time);
    }
    else
    {
        syslog(LOG_NOTICE, "Inactive session will be kicked after %ld minutes", kick_time);
    }

    int q_cnt = 3, q_size = 150, ind = 0;
    char **queries = malloc(sizeof(char) * q_cnt * q_size);

    char ip_address[15], *nl;

    for(;;)
    {
        // get source ip from squid
        fgets(ip_address, 15, stdin);

        nl = strrchr(ip_address, '\n');
        if(nl) *nl = '\0';

        // remove all inactive sessions from database
        queries[ind++] = sqlite3_mprintf("DELETE FROM scd_sessions WHERE `last_visit`<%ld", time(NULL) - (kick_time * 60));
        // DEBUG
        //queries[ind++] = sqlite3_mprintf("INSERT INTO scd_sessions VALUES ('192.168.1.2', 'nefarius', %ld)", time(NULL));
        retval = sqlite3_exec(db_con, queries[ind - 1], 0, 0, 0);
        if(retval) syslog(LOG_WARNING, "sqlite3: %s\n", sqlite3_errmsg(db_con));
        // get active session username
        queries[ind++] = sqlite3_mprintf("SELECT user FROM scd_sessions WHERE `ip`=\"%s\"", ip_address);
        retval = sqlite3_prepare_v2(db_con,queries[ind - 1], -1, &db_stmt, 0);
        if(retval) syslog(LOG_WARNING, "sqlite3: %s\n", sqlite3_errmsg(db_con));

        retval = sqlite3_step(db_stmt);

        if(retval == SQLITE_ROW)
        {
            const char *username = (const char*)sqlite3_column_text(db_stmt, 0);
            printf("OK user=%s\n", username);

            queries[ind++] = sqlite3_mprintf("UPDATE scd_sessions SET `last_visit`=%ld WHERE `ip`=\"%s\"", time(NULL), ip_address);
            retval = sqlite3_exec(db_con, queries[ind - 1], 0, 0, 0);
            if(retval) syslog(LOG_WARNING, "sqlite3: %s\n", sqlite3_errmsg(db_con));
        }
        else
        {
            printf("ERR\n");
        }

        ind = 0;
    }

    clean_shutdown(&cfg, db_con);
    return 0;
}
