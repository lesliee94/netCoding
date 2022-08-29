#include "./mycs.h"
using namespace std;
//INET_ADDRSTRLEN  16
//INET6_ADDRSTRLEN 46

/* for processes*/

void err(const string s){
	cerr<<s<<"  error: "<<strerror(errno)<<endl;
	exit(errno);
}
pid_t Fork(void)
{
    pid_t pid;

    if ((pid = fork()) < 0)
        err("Fork");
    return pid;
}
pid_t Waitpid(pid_t pid, int *iptr, int options)
{
    pid_t retpid;
    if ((retpid  = waitpid(pid, iptr, options)) < 0)
        err("Waitpid");
    return(retpid);
}
void Kill(pid_t pid, int signum)
{
    int rc;

    if ((rc = kill(pid, signum)) < 0)
        err("Kill");
}

void Execve(const char *filename, char *const argv[], char *const envp[])
{
    if (execve(filename, argv, envp) < 0)
        err("Execve");
}
//void Execlp(const char*file,const char* argc[]){//end with nullptr
//	if(execlp(file,argc,...,nullptr)<0)
//		err("Execlp");
//}
//void Execl(const char *path, const char *arg[]){
//	if(execl(path,arg,...,nullptr)<0)
//		err("Execl");
//}

pid_t Wait(int *status)
{
    pid_t pid;

    if ((pid  = wait(status)) < 0)
        err("Wait");
    return pid;
}
void Pause(){
	(void)pause();
	return;
}
unsigned int Sleep(unsigned int secs){
	unsigned int rc;
	if((rc=sleep(secs))<0){
		err("sleep");
	}
	return rc;
}
unsigned int Alarm(unsigned int seconds) {
    return alarm(seconds);
}

void Setpgid(pid_t pid, pid_t pgid) {
    int rc;

    if ((rc = setpgid(pid, pgid)) < 0)
        err("Setpgid");
    return;
}

pid_t Getpgrp(void) {
    return getpgrp();
}




/* end processes */

/* start signal  */
handler_t *Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        err("Signal");
    return (old_action.sa_handler);
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if (sigprocmask(how, set, oldset) < 0)
        err("Sigprocmask");
    return;
}

void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0)
        err("Sigemptyset");
    return;
}

void Sigfillset(sigset_t *set)
{
    if (sigfillset(set) < 0)
        err("Sigfillset");
    return;
}

void Sigaddset(sigset_t *set, int signum)
{
    if (sigaddset(set, signum) < 0)
        err("Sigaddset");
    return;
}

void Sigdelset(sigset_t *set, int signum)
{
    if (sigdelset(set, signum) < 0)
        err("Sigdelset");
    return;
}

int Sigismember(const sigset_t *set, int signum)
{
    int rc;
    if ((rc = sigismember(set, signum)) < 0)
        err("Sigismember");
    return rc;
}

/*end signal*/

/*begin socket*/

int Socket(int domain,int type,int protocal){
     int sockfd=-1;
     switch(domain){
         case AF_INET:
             cout<<"ipv4 "<<endl;
             sockfd=socket(AF_INET,type,protocal);
             break;
         case AF_INET6:
             cout<<"ipv6"<<endl;
             sockfd=socket(AF_INET6,type,protocal);
             break;
         default:
             cout<<"not defined yet!"<<endl;
             break;
     }
      if(sockfd==-1){
		  err("socket");
     }
     return sockfd;
 }
void Setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
    int rc;

    if ((rc = setsockopt(s, level, optname, optval, optlen)) < 0)
        err("Setsockopt");
}

void ServInit(struct sockaddr_in&ser,sockaddr_in&cli){
	ser.sin_addr.s_addr=htonl(INADDR_ANY);
	ser.sin_port=htons(PORT);
	ser.sin_family=AF_INET;
	bzero(&cli,sizeof(cli));
	cli.sin_family=AF_INET;
}
void ServInit(struct sockaddr_in6&ser,struct sockaddr_in6&cli){
	ser.sin6_addr=in6addr_any;
	ser.sin6_port=htons(PORT);
	ser.sin6_family=AF_INET6;
	bzero(&cli,sizeof(cli));
	cli.sin6_family=AF_INET6;
}

int Bind(int& sockfd,struct sockaddr_in&ser){
	int ret =bind(sockfd,(SA*)&ser,sizeof(ser));
	 if(ret==-1){
		 err("bind");
	 }
	 return ret;
}
int Bind(int& sockfd,struct sockaddr_in6&ser){
	int ret=bind(sockfd,(SA*)&ser,sizeof(ser));
	if(ret==-1){
		err("bind");
	}
	return ret;
}

int Listen(int& sockfd,int backlog){
	int ret=listen(sockfd,backlog);
	if(ret==-1){
		err("listen"); 
	}
     return ret;
}

int Accept(int&lsfd,struct sockaddr_in6&cli,socklen_t &addrlen){
	int confd;
again:
	confd=accept(lsfd,(SA*)&cli,&addrlen);
	if(confd==-1){
		if(errno==ECONNABORTED||errno==EINTR){
			goto again;
		}
		else{
			err("accept");
		}
	}
	return confd;
}
 int Accept(int&lsfd,struct sockaddr_in&cli,socklen_t &addrlen){
    int confd;
 again:
    confd=accept(lsfd,(SA*)&cli,&addrlen);
    if(confd==-1){
        if(errno==ECONNABORTED||errno==EINTR){
            goto again;
        }
        else{
			err("accept");
		}
    }
    return confd;
}   
int  ClienInit(struct sockaddr_in&ser,const char* &ip,int clienfd){
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(PORT);
	int ret=inet_pton(AF_INET,ip,&ser.sin_addr);
	if(ret!=1){
		err("pton");
	}
	return ret;
}
// int inet_pton(int af, const char *src, void *dst);

 int ClienInit(struct sockaddr_in6&ser,const char* &ip,int clienfd){
    bzero(&ser,sizeof(ser));
    ser.sin6_family=AF_INET6;
    ser.sin6_port=htons(PORT);
    int ret=inet_pton(AF_INET6,ip,&ser.sin6_addr);
	if(ret!=1){
		err("ntop");
	}
	return ret;
}     
int Connect(int& clifd,struct sockaddr_in&ser){
	int ret=connect(clifd,(SA*)&ser,sizeof(ser));
	if(ret==-1){
		err("connect");
	}
	return ret;
}

 int Connect(int& clifd,struct sockaddr_in6&ser){
    int ret=connect(clifd,(SA*)&ser,sizeof(ser));
    if(ret==-1){
		err("connect");
    }
    return ret;
}
int Close(int fd){
	int ret=close(fd);
	if(ret==-1){
		 err("close");
	}
	return ret;
}
//dns wrappers
//
struct hostent *Gethostbyname(const char *name)
{
    struct hostent *p;

    if ((p = gethostbyname(name)) == NULL)
       err("Gethostbyname");
    return p;
}
/* $end gethostbyname */

struct hostent *Gethostbyaddr(const char *addr, int len, int type)
{
    struct hostent *p;

    if ((p = gethostbyaddr(addr, len, type)) == NULL)
        err("Gethostbyaddr");
    return p;
}

//files

int Open(const char *pathname, int flags, mode_t mode)
{
    int rc;

    if ((rc = open(pathname, flags, mode))  < 0)
        err("Open");
    return rc;
}

ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t rc;

    if ((rc = read(fd, buf, count)) < 0)
        err("Read");
    return rc;
}

ssize_t Write(int fd, const void *buf, size_t count)
{
    ssize_t rc;

    if ((rc = write(fd, buf, count)) < 0)
        err("Write");
    return rc;
}

off_t Lseek(int fildes, off_t offset, int whence)
{
    off_t rc;

    if ((rc = lseek(fildes, offset, whence)) < 0)
        err("Lseek");
    return rc;
}

int Select(int  n, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout)
{
    int rc;

    if ((rc = select(n, readfds, writefds, exceptfds, timeout)) < 0)
        err("Select");
    return rc;
}

int Dup2(int fd1, int fd2)
{
    int rc;

    if ((rc = dup2(fd1, fd2)) < 0)
        err("Dup2");
    return rc;
}

void Stat(const char *filename, struct stat *buf)
{
    if (stat(filename, buf) < 0)
        err("Stat");
}

void Fstat(int fd, struct stat *buf)
{
    if (fstat(fd, buf) < 0)
        err("Fstat");
}

// memory functions

void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    void *ptr;

    if ((ptr = mmap(addr, len, prot, flags, fd, offset)) == ((void *) -1))
        err("mmap");
    return(ptr);
}

void Munmap(void *start, size_t length)
{
    if (munmap(start, length) < 0)
        err("munmap");
}
// Ctype I/O
void Fclose(FILE *fp)
{
    if (fclose(fp) != 0)
       err("Fclose");
}

FILE *Fdopen(int fd, const char *type)
{
    FILE *fp;

    if ((fp = fdopen(fd, type)) == NULL)
        err("Fdopen");

    return fp;
}

char *Fgets(char *ptr, int n, FILE *stream)
{
    char *rptr;

    if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
        err("Fgets");

    return rptr;
}

FILE *Fopen(const char *filename, const char *mode)
{
    FILE *fp;

    if ((fp = fopen(filename, mode)) == NULL)
        err("Fopen");

    return fp;
}
void Fputs(const char *ptr, FILE *stream)
{
    if (fputs(ptr, stream) == EOF)
        err("Fputs");
}

size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t n;

    if (((n = fread(ptr, size, nmemb, stream)) < nmemb) && ferror(stream))
        err("Fread");
    return n;
}

void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (fwrite(ptr, size, nmemb, stream) < nmemb)
        err("Fwrite");
}


/* rio */
//unbuffered read n bytes
//
ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp =(char*) usrbuf;

    while (nleft > 0) {
        if ((nread =read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) /* Interrupted by sig handler return */
                nread = 0;      /* and call read() again */
            else
                return -1;      /* errno set by read() */
        }
        else if (nread == 0)
            break;              /* EOF */
        nleft -= nread;      //already read nread 
        bufp += nread;      //so ptr add nread
    }
    return (n - nleft);         /* return >= 0 */
}
// On  success,  the  number of bytes read is returned (zero indicates end of file)  READ
/*
 * rio_writen - robustly write n bytes (unbuffered)
 */
/* $begin rio_writen */
ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp =(char*) usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR)  /* Interrupted by sig handler return */
                nwritten = 0;    /* and call write() again */
            else
                return -1;       /* errno set by write() */
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}
/* 
 * rio_read - This is a wrapper for the Unix read() function that
 *    transfers min(n, rio_cnt) bytes from an internal buffer to a user
 *    buffer, where n is the number of bytes requested by the user and
 *    rio_cnt is the number of unread bytes in the internal buffer. On
 *    entry, rio_read() refills the internal buffer via a call to
 *    read() if the internal buffer is empty.
 */
/* $begin rio_read */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  /* Refill if buf is empty */
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
                           sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR) /* Interrupted by sig handler return */
                return -1;
        }
        else if (rp->rio_cnt == 0)  /* EOF */
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;
    if (rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    bcopy(rp->rio_bufptr,usrbuf,cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

/* $begin rio_readnb */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp =(char*) usrbuf;

    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0)
            return -1;          /* errno set by read() */
        else if (nread == 0)
            break;              /* EOF */
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}
/* $end rio_readnb */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c;
	char*bufp =(char*) usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;
            }
        } else if (rc == 0) {
            if (n == 1)
                return 0; /* EOF, no data read */
            else
                break;    /* EOF, some data was read */
        } else
            return -1;    /* Error */
    }
    *bufp = 0;
    return n-1;
}
/* $end rio_readlineb */
/**********************************
 * Wrappers for robust I/O routines
 **********************************/
ssize_t Rio_readn(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

    if ((n = rio_readn(fd, ptr, nbytes)) < 0)
        err("Rio_readn");
    return n;
}

void Rio_writen(int fd, void *usrbuf, size_t n)
{
    if (rio_writen(fd, usrbuf, n) != n)
        err("Rio_writen");
}

void Rio_readinitb(rio_t *rp, int fd)
{
    rio_readinitb(rp, fd);
}

ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    ssize_t rc;

    if ((rc = rio_readnb(rp, usrbuf, n)) < 0)
        err("Rio_readnb");
    return rc;
}
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
        err("Rio_readlineb");
    return rc;
}

/******************************** 
 * Client/server helper functions
 ********************************/
/*
 * open_clientfd - open connection to server at <hostname, port> 
 *   and return a socket descriptor ready for reading and writing.
 *   Returns -1 and sets errno on Unix error. 
 *   Returns -2 and sets h_errno on DNS (gethostbyname) error.
 */
/* $begin open_clientfd */
int open_clientfd(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1; /* Check errno for cause of error */

    /* Fill in the server's IP address and port */
    if ((hp = gethostbyname(hostname)) == NULL)
        return -2; /* Check h_errno for cause of error */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
          (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}
/* $end open_clientfd */

/*  
 * open_listenfd - open and return a listening socket on port
 *     Returns -1 and sets errno on Unix error.
 */
/* $begin open_listenfd */
int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
}
/* $end open_listenfd */
/******************************************
 * Wrappers for the client/server helper routines 
 ******************************************/
int Open_clientfd(char *hostname, int port)
{
    int rc;

    if ((rc = open_clientfd(hostname, port)) < 0) {
        if (rc == -1)
           err("Open_clientfd");
        else
            err("dns Open_clientfd");
    }
    return rc;
}

int Open_listenfd(int port)
{
    int rc;

    if ((rc = open_listenfd(port)) < 0)
       err("Open_listenfd");
    return rc;
}

/************************************************
 * Wrappers for Pthreads thread control functions
 ************************************************/

void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
                    void * (*routine)(void *), void *argp)
{
    int rc;

    if ((rc = pthread_create(tidp, attrp, routine, argp)) != 0)
        err("Pthread_create");
}

void Pthread_cancel(pthread_t tid) {
    int rc;

    if ((rc = pthread_cancel(tid)) != 0)
        err("Pthread_cancel");
}

void Pthread_join(pthread_t tid, void **thread_return) {
    int rc;

    if ((rc = pthread_join(tid, thread_return)) != 0)
        err("Pthread_join");
}

/* $begin detach */
void Pthread_detach(pthread_t tid) {
    int rc;

    if ((rc = pthread_detach(tid)) != 0)
        err("Pthread_detach");
}
/* $end detach */
void Pthread_exit(void *retval) {
    pthread_exit(retval);
}

pthread_t Pthread_self(void) {
    return pthread_self();
}

void Pthread_once(pthread_once_t *once_control, void (*init_function)()) {
    pthread_once(once_control, init_function);
}

/*******************************
 * Wrappers for Posix semaphores
 *******************************/
void Sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (sem_init(sem, pshared, value) < 0)
        err("Sem_init");
}

void P(sem_t *sem)
{
    if (sem_wait(sem) < 0)
        err("P");
}

void V(sem_t *sem)
{
    if (sem_post(sem) < 0)
        err("V");
}

