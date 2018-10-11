// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#ifndef KBE_HTTP_UTILTY_H
#define KBE_HTTP_UTILTY_H

#include "common/common.h"
#include "common/timer.h"

namespace KBEngine 
{
class MemoryStream;
namespace Network
{
namespace Http
{ 
	bool initialize();
	void finalise();

    inline uint8 toHex(const uint8 &x)
    {
        return x > 9 ? x -10 + 'A': x + '0';
    }

    inline uint8 fromHex(const uint8 &x)
    {
        return isdigit(x) ? x-'0' : x-'A'+10;
    }
 
    inline std::string URLEncode(const std::string &sIn)
    {
        std::string sOut;
        
        for( size_t ix = 0; ix < sIn.size(); ix++ )
        {      
            uint8 buf[4];
            memset( buf, 0, 4 );
            
            if( isalnum( (uint8)sIn[ix] ) )
            {      
                buf[0] = sIn[ix];
            }
            //else if ( isspace( (uint8)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
            //{
            //    buf[0] = '+';
            //}
            else
            {
                buf[0] = '%';
                buf[1] = toHex( (uint8)sIn[ix] >> 4 );
                buf[2] = toHex( (uint8)sIn[ix] % 16);
            }
            
            sOut += (char *)buf;
        }
        
        return sOut;
    };

    inline std::string URLDecode(const std::string &sIn)
    {
        std::string sOut;
        
        for( size_t ix = 0; ix < sIn.size(); ix++ )
        {
            uint8 ch = 0;
            if(sIn[ix]=='%')
            {
                ch = (fromHex(sIn[ix+1])<<4);
                ch |= fromHex(sIn[ix+2]);
                ix += 2;
            }
            else if(sIn[ix] == '+')
            {
                ch = ' ';
            }
            else
            {
                ch = sIn[ix];
            }
            
            sOut += (char)ch;
        }
        
        return sOut;
    }

	class Request
	{
	public:
		enum Status {
			OK = 0,
			INVALID_OPT = 1,
			PERFORM_ERROR = 2,
			OPENFILE_ERROR = 3,
			INIT_ERROR = 4,
		};

		/* 
			success, data
		*/
		typedef std::function<void(bool, const Request& pRequest, const std::string&)> Callback;

	public:
		Request();
		~Request();

		Status setURL(const std::string& url);

		/* example.com is redirected, so we tell libcurl to follow redirection */
		Status setFollowURL(int maxRedirs);

		Status setPostData(const void* data, unsigned int size);

		Status setTimeout(uint32 time);

		Status setHeader(const std::vector<std::string>& headers);
		Status setHeader(const std::string& header);

		Status setProxy(const std::string& proxyIP, long proxyPort);

		Status setCallback(const Callback& resultCallback) {
			resultCallback_ = resultCallback;
			return OK;
		}

		void callCallback(bool success);

		Status perform();

		int getHttpCode() { return httpCode_; }
		const char* getReceivedHeader() { return receivedHeader_.c_str(); }
		const char* getReceivedContent() { return receivedContent_.c_str(); }
		const char* getError() { return (const char*)&error_[0]; }

		static size_t receiveHeaderFunction(char *buffer, size_t size, size_t nitems, void *userdata);
		static size_t receiveContentFunction(char *ptr, size_t size, size_t nmemb, void *userdata);

		void* pContext() {
			return pContext_;
		}

	private:
		void* pContext_;
		void* headers_;
		MemoryStream* postData_;
		long httpCode_;
		char error_[MAX_BUF];
		bool hasSetRedirect_;
		int retryTimes_;

		std::string receivedContent_;
		std::string receivedHeader_;

		Callback resultCallback_;
		bool called_;
	};

	class Requests : public TimerHandler
	{
	public:
		Requests();
		~Requests();

		/*
			异步http请求
		*/
		Request::Status perform(Request* pRequest);
		Request::Status perform(const std::string& url, const Request::Callback& resultCallback);

		void* pContext() {
			return pContext_;
		}

		virtual void handleTimeout(TimerHandle, void * pUser);

	public:
		int still_running;
		Request* pRequest;
		TimerHandle timerHandle;

	private:
		void* pContext_;

	};

	static Request::Status perform(Request* pRequest);
	static Request::Status perform(const std::string& url, const Request::Callback& resultCallback);

}
}
}

#endif // KBE_HTTP_UTILTY_H


