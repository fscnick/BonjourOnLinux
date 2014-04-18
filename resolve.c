#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <netdb.h>

#include "dns_sd.h"

char *discoveredName;
char *discoveredRegtype;
char *discoveredDomain;

uint16_t endianChange(uint16_t i)
{
    return ((i>>8)&0xff)+((i<<8)&0xff00);
}

void customBrowseReply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
			    const char *serviceName, const char *regtype, const char *replyDomain, void *context)
{
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


    discoveredName=malloc(strlen(serviceName)+1);
    strcpy(discoveredName, serviceName);

    discoveredRegtype=malloc(strlen(regtype)+1);
    strcpy(discoveredRegtype, regtype);
    
    discoveredDomain=malloc(strlen(replyDomain)+1);
    strcpy(discoveredDomain, replyDomain);

}

void customResolveReply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
			    const char *fullname, const char *hosttarget, uint16_t port, /* In network byte order */ 
			    uint16_t txtLen, const unsigned char *txtRecord, void *context)
{

    struct hostent *hostInfo;
    struct in_addr *serv_addr;


    if ( (hostInfo=gethostbyname(hosttarget))== NULL){
	printf("Call gethostbyname ERROR!!");
	
    }else{
	serv_addr=(struct in_addr *) hostInfo->h_addr_list[0]; 	

	printf("In ResolveReply callback!!\n");
	printf("full name: %s\n", fullname);
	printf("host target: %s\n", hosttarget);
	printf("host ip addr: %s\n", inet_ntoa(*serv_addr));
	printf("port: %d\n", endianChange(port));
	printf("txtLen: %d\n", txtLen);

	char *temp=malloc(txtLen+1);
	strncpy(temp, txtRecord, txtLen);
	temp[txtLen]='\0';
	printf("txtRecord: %s\n", temp);

    }

}

int main(int argc, char* argv[])
{
    DNSServiceRef browseRef;
    DNSServiceRef resolveRef;
    DNSServiceErrorType result;
    DNSServiceFlags nativeFlag=0;

    int port = 11234;
    char* serviceType="_personal._tcp";
    char* netInterface="eth0";
    int netIndex=if_nametoindex(netInterface);
    int serviceFD;



    char input;

    printf("Starting to Discover service...\n");


    result=DNSServiceBrowse(
			&browseRef, 
			nativeFlag,
			netIndex,
			serviceType,
			NULL,	/* domain */
			&customBrowseReply,	/* callback */
			NULL	/* context, user-defined that will be an argument of callback */
			);

    if (result == kDNSServiceErr_NoError){
	printf("Browse service success!!\n");	
    }else{
	printf("Browser service fail!!\n");
	printf("Error code: %d\n", result);
	return 1;
    }
    //serviceFD=DNSServiceRefSockFD(browseRef);
    // this method will active the callback register before.
    // TO-DO if no service is available, will block the program. solve it.
    result=DNSServiceProcessResult( browseRef );


    if (result == kDNSServiceErr_NoError){
	printf("Browse response success!!\n");

	printf("discovered:\n");
	printf("%s %s %s", discoveredName, discoveredRegtype, discoveredDomain);


    }else{
	printf("Browse response fail!!\n");
	printf("Error code: %d\n", result);
    	return 1;
    }


    result=DNSServiceResolve(
			&resolveRef,
			nativeFlag,
			netIndex,
			discoveredName,
			discoveredRegtype,
			discoveredDomain,
			&customResolveReply,	    /* callback */
			NULL	    /* context, user-defined that will be an argument of callback */
			);

    if (result == kDNSServiceErr_NoError){
	printf("Resolve service success!!\n");
    }else{
	printf("Resolve service fail!!\n");
	printf("Error code: %d\n", result);
    	return 1;
    }
    
    result=DNSServiceProcessResult(resolveRef);

    
    if (result == kDNSServiceErr_NoError){
	printf("Resolve response success!!\n");
    }else{
	printf("Rosolve response fail!!\n");
	printf("Error code: %d\n", result);
    	return 1;
    }


    
    printf("press 'q' to exist:\n");
    while ((input=getchar()) != -1 && input!='q');


    DNSServiceRefDeallocate(resolveRef);
    // this must be call before calling DNSServiceBrowser again.
    DNSServiceRefDeallocate(browseRef);
    
    free(discoveredName);
    free(discoveredRegtype);
    free(discoveredDomain);

    return 0;

}
