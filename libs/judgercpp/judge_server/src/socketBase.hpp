/**
 * @desc 封装socket的基本操作
 */
#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class socketBase {
    public:
        /**
         * @desc 读取n个字节数字
         */
        static bool readn(const int sockfd,char * buffer,const size_t n);
        /**
         * @desc 写入n个字节数字
         */
        static bool writen(const int sockfd,const char * buffer,const size_t n);

        static bool TcpRead(const int sockfd,std::string& buffer,int *ibuflen,const int itimeout=0);
        static bool TcpWrite(const int sockfd,const char *buffer,const int ibuflen);

};

bool socketBase::readn(const int sockfd,char * buffer,const size_t n)
{
    int nLeft,nread,idx;
    nLeft = n;
    idx   = 0;
    while(nLeft > 0)
    {
        if ( (nread = recv(sockfd,buffer + idx,nLeft,0)) <= 0) return false;
        idx += nread;
        nLeft -= nread;
    }
    return true;
}

bool socketBase::writen(const int sockfd,const char * buffer,const size_t n)
{
    int nLeft,idx,nwritten;
    nLeft = n;
    idx = 0;
    while(nLeft > 0 )
    {
        if ( (nwritten = send(sockfd, buffer + idx,nLeft,0)) <= 0) return false;

        nLeft -= nwritten;
        idx += nwritten;
    }

    return true;
}

bool socketBase::TcpRead(const int sockfd,std::string& buffer,int *ibuflen,const int itimeout)
{
    if (sockfd == -1) return false;

    if (itimeout > 0)
    {
        fd_set tmpfd;

        FD_ZERO(&tmpfd);
        FD_SET(sockfd,&tmpfd);

        struct timeval timeout;
        timeout.tv_sec = itimeout; timeout.tv_usec = 0;

        int i;
        if ( (i = select(sockfd+1,&tmpfd,0,0,&timeout)) <= 0 ) return false;
    }

    (*ibuflen) = 0;

    if (readn(sockfd,(char*)ibuflen,4) == false) return false;

    (*ibuflen)=ntohl(*ibuflen);  // 把网络字节序转换为主机字节序。

    buffer.resize(*ibuflen);
    if (readn(sockfd,buffer.data(),(*ibuflen)) == false) return false;

    return true;
}

bool socketBase::TcpWrite(const int sockfd,const char *buffer,const int ibuflen)
{
  if (sockfd == -1) return false;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(sockfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = 5; timeout.tv_usec = 0;

  if ( select(sockfd+1,0,&tmpfd,0,&timeout) <= 0 ) return false;
  
  int ilen=0;

  // 如果长度为0，就采用字符串的长度
  if (ibuflen==0) ilen=strlen(buffer);
  else ilen=ibuflen;

  int ilenn=htonl(ilen);  // 转换为网络字节序。

  //char strTBuffer[ilen+4];
  std::string strTBuffer;
  strTBuffer.resize(ilen+4);
  memset(strTBuffer.data(),0,sizeof(strTBuffer));
  memcpy(strTBuffer.data(),&ilenn,4);
  memcpy(strTBuffer.data()+4,buffer,ilen);
  
  if (writen(sockfd,strTBuffer.c_str(),ilen+4) == false) return false;

  return true;
}
