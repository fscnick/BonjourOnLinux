#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>

#include "dns_sd.h"


struct BrowseReplyData{
    DNSServiceErrorType errorCode;
    char *serviceName;
    char *regType;
    char *replyDomain;
    };

void customBrowseReply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
			    const char *serviceName, const char *regtype, const char *replyDomain, void *context)
{
	// init BrowseReplyData
    struct BrowseReplyData *replyData=context;
    replyData->errorCode=errorCode;
    replyData->serviceName=malloc(strlen(serviceName)+1);
    replyData->regType=malloc(strlen(regtype)+1);
    replyData->replyDomain=malloc(strlen(replyDomain)+1);
	
	strcpy(replyData->serviceName, serviceName);
	strcpy(replyData->regType, regtype);
	strcpy(replyData->replyDomain, replyDomain);


    printf("In BrowseReply callback!!\n");
    if(flags == kDNSServiceFlagsMoreComing){
	printf("flag: kDNSServiceFlagsMoreComing\n");
    }else if(flags == kDNSServiceFlagsAdd){
	printf("flag: kDNSServiceFlagsAdd\n");
    }else{
	printf("flag: %d\n", flags);
    }
    printf("Error Code: %d\n", errorCode);
    printf("Service Name: %s\n", serviceName);
    printf("RegType: %s\n", regtype);
    printf("Domain: %s\n", replyDomain);



}

int main(int argc, char* argv[])
{
    DNSServiceRef ref;
    DNSServiceErrorType result;
    DNSServiceFlags nativeFlag=0;
	
	struct BrowseReplyData *replyData;

    int port = 11234;
    char* serviceType="_personal._tcp";

    char* netInterface="eth0";
    int netIndex=if_nametoindex(netInterface);
    int serviceFD;

    char input;

    printf("Starting to Discover service...\n");


    result=DNSServiceBrowse(
			&ref, 
			nativeFlag,
			netIndex,
			serviceType,
			NULL,	/* domain */
			&customBrowseReply,	/* callback */
			replyData	/* context, user-defined that will be an argument of callback */
			);

    if (result == kDNSServiceErr_NoError){
	printf("Browse service success!!\n");	
    }else{
	printf("Browser service fail!!\n");
	printf("Error code: %d\n", result);
	return 1;
    }
    //serviceFD=DNSServiceRefSockFD(ref);
    // this method will active the callback register before.
    // TO-DO if no service is available, will block the program. solve it.
    result=DNSServiceProcessResult( ref );


    if (result == kDNSServiceErr_NoError){
		printf("Browse response success!!\n");	
	
		printf("%s %s %s\n", replyData->serviceName, replyData->regType, replyData->replyDomain);
    }else{
		printf("Browse response fail!!\n");
		printf("Error code: %d\n", result);
    	return 1;
    }

    printf("press 'q' to exist:\n");
    while ((input=getchar()) != -1 && input!='q');

    // this must be call before calling DNSServiceBrowser again.
    DNSServiceRefDeallocate(ref);
    
    return 0;

}
