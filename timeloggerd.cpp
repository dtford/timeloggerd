#include <ctime>
#include <syslog.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

#define OK 0
#define ERR_SETSID 40
#define ERR_CHDIR 41
#define ERR_WTF 42
#define DAEMON_NAME "timeloggerd"
#define ERROR_FORMAT 	"ERROR: %s"
#define INFO_FORMAT		"INFO: %s"

static void _do_work( )
{
	while( 1 )
	{
		time_t rawtime;
		struct tm *timeinfo;
		time( &rawtime );
		timeinfo = localtime( &rawtime );
		syslog( LOG_INFO, INFO_FORMAT, asctime( timeinfo ) );
		sleep(1);
	}
}

static void _signal_handler( const int signal )
{
	switch(signal)
	{
		case SIGHUP:
		{
			break;
		}
		case SIGTERM:
		{
			syslog( LOG_INFO, INFO_FORMAT, "received SIGTERM, exiting." );
			closelog();
			exit( OK );
			break;
		}
		default:
		{
			syslog( LOG_INFO, INFO_FORMAT, "received unhandled signal." );
			break;
		}
	}
}

int main(void)
{
	openlog( DAEMON_NAME, LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON );
	syslog( LOG_INFO, INFO_FORMAT, "starting timeloggerd");

	// fork, so that we don't take over the initializer
	pid_t pid = fork();

	// check value of fork
	if( pid < 0 ) // something went wrong.
	{
		syslog( LOG_ERR, ERROR_FORMAT, strerror(errno) );
	}
	if( pid > 0 ) // we are the parent process, so just exit
	{
		return OK;
	}

	// if we get here, we're the child session, so we keep going
	if( setsid() < -1 )
	{
		syslog( LOG_ERR, ERROR_FORMAT, strerror(errno) );
		return ERR_SETSID;
	}

	close( STDIN_FILENO );
	close( STDOUT_FILENO );
	close( STDERR_FILENO );

	umask( S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

	if( chdir("/") < 0 )
	{
		syslog(LOG_ERR, ERROR_FORMAT, strerror(errno) );
		return ERR_CHDIR;
	}

	// setup signal handling
	signal( SIGTERM, _signal_handler );
	signal (SIGHUP, _signal_handler );

	_do_work();

	return ERR_WTF;
}
