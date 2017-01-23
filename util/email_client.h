//
// Created by liaosiwei on 17/1/5.
//

#ifndef OKCOIN_BOT_EMAIL_CLIENT_H
#define OKCOIN_BOT_EMAIL_CLIENT_H

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

/*
 * SMTPClient mailc("yoursmtpserver.com",25,"user@yourdomain.com","password");
 * mailc.Send("from@yourdomain.com","to@somewhere.com","subject","Hello from C++ SMTP Client!");
 */
namespace okcoin_email {
    using boost::asio::ip::tcp;
    using namespace boost::archive::iterators;
    typedef base64_from_binary<transform_width<const char *,6,8> > base64_text;

    class SMTPClient
    {
    public:


        SMTPClient(std::string pServer,unsigned int pPort,std::string pUser,std::string pPassword):
                mServer(pServer),mPort(pPort),mUserName(pUser),mPassword(pPassword),mSocket(mIOService),mResolver(mIOService)
        {
            tcp::resolver::query qry(mServer,boost::lexical_cast<std::string>( mPort ));
            mResolver.async_resolve(qry,boost::bind(&SMTPClient::handleResolve,this,boost::asio::placeholders::error,
                                                    boost::asio::placeholders::iterator));
        }
        bool Send(std::string pFrom,std::string pTo,std::string pSubject,std::string pMessage)
        {
            mFrom=pFrom;
            mTo=pTo;
            mSubject=pSubject;
            mMessage=pMessage;
            mIOService.run();
            return mHasError;
        }
        bool hasErr() const {
            return mHasError;
        }
        std::string getErr() const {
            return mErrorMsg;
        }
    private:
        std::string encodeBase64(std::string pData)
        {
            std::stringstream os;
            size_t sz=pData.size();
            std::copy(base64_text(pData.c_str()),base64_text(pData.c_str()+sz),std::ostream_iterator<char>(os));
            return os.str();
        }
        void handleResolve(const boost::system::error_code& err,tcp::resolver::iterator endpoint_iterator)
        {
            if(!err)
            {
                tcp::endpoint endpoint=*endpoint_iterator;
                mSocket.async_connect(endpoint,
                                      boost::bind(&SMTPClient::handleConnect,this,boost::asio::placeholders::error,++endpoint_iterator));
            }
            else
            {
                mHasError=true;
                mErrorMsg= err.message();
            }
        }
        void writeLine(std::string pData)
        {
            std::ostream req_strm(&mRequest);
            req_strm << pData << "\r\n";
            boost::asio::write(mSocket,mRequest);
            req_strm.clear();
        }
        void handleConnect(const boost::system::error_code& err,tcp::resolver::iterator endpoint_iterator)
        {
            if (!err)
            {
                // The connection was successful. Send the request.
                std::ostream req_strm(&mRequest);
                writeLine("EHLO "+mServer);
                writeLine("AUTH LOGIN");
                writeLine(encodeBase64(mUserName));
                writeLine(encodeBase64(mPassword));
                writeLine( "MAIL FROM:<"+mFrom+">");
                writeLine( "RCPT TO:<"+mTo+">");
                writeLine( "DATA");
                writeLine( "SUBJECT:"+mSubject);
                writeLine( "From:"+mFrom);
                writeLine( "To:"+mTo);
                writeLine( "");
                writeLine( mMessage );
                writeLine( ".\r\n");
            }
            else
            {
                mHasError=true;
                mErrorMsg= err.message();
            }
        }
        std::string mServer;
        std::string mUserName;
        std::string mPassword;
        std::string mFrom;
        std::string mTo;
        std::string mSubject;
        std::string mMessage;
        unsigned int mPort;
        boost::asio::io_service mIOService;
        tcp::resolver mResolver;
        tcp::socket mSocket;
        boost::asio::streambuf mRequest;
        boost::asio::streambuf mResponse;
        bool mHasError;
        std::string mErrorMsg;
    };
}
#endif //OKCOIN_BOT_EMAIL_CLIENT_H
