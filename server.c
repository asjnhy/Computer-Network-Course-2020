/*
<<웹서버 c언어 이용하여 구현하기- Server side>>
- 컴퓨터 네트워크 과제
2016003654 안채령
*/


/*
필요한 헤더 파일 include 
-->sys/socket.h : 소켓 통신
-->netinet/in.h : 인터넷 프로토콜
-->fcntl.h : 파일의 open, read 등을 위한 헤더 
-->sys/types.h : socket.h 와 netinet/in.h 에 쓰이는 데이터타입 정의
*/
#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>




int main(int argc, char *argv[]) {

	int sockfd, newsockfd, portno, n; //sockfd: 서버의 소켓filedescriptor, newsockfd:sockfd에서 accept되어 생성되는 소켓 , portno: 포트넘버, n: read의 성공적인리턴 확인 위함
	socklen_t clilen;//	clilen(cliaddr의 사이즈) 정의
	char buffer[512];//socket이 클라이언트로부터 받는 메세지 buffer 에 저장 
	char lengthbuf[128];//request field 중 content-length 를 넣기 위해 정의 
	struct sockaddr_in serv_addr, cli_addr; //Internet Address 정보를 가진 구조체 - 서버측, 클라이언트 측 생성 

	
	FILE *file; //바이너리로 fopen 하는 파일을 담기 위한 변수
	unsigned char *buf;//열게되는 파일들을 담기위한 buffer 
	unsigned long fileLen;//열게되는 파일들의 len


	if(argc<2){ //arg의 수는 1개(포트번호 입력)
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	/*#1. socket(): socket() 시스템 콜로 소켓 생성:sockfd
	실패시 -1 반환 */
	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0) error("ERROR opening socket");


	bzero((char*) &serv_addr, sizeof(serv_addr));//serv_addr 초기화
	portno= atoi(argv[1]); //받은 argv(portno)를 str to int 로 변환
	serv_addr.sin_family= AF_INET;//IPv4 주소체계를 이용 
	serv_addr.sin_addr.s_addr= htonl(INADDR_ANY);// host byte order에서 network byte order로 바꿈, 이때 INADDR_ANY 는 서버의 IP 주소를 자동으로 찾아서 대입해줌
	serv_addr.sin_port = htons(portno);//networkbyte order 로 바꾸고 할당 


	/*#2. bind() : 소켓을 identify 해줌
	(IP와 포트 넘버를 bind()시스템 콜을 이용하여 할당해줌)
	실패시 -1 반환 */
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");

	/*#3. listen( ): 연결을 위해 기다리고 있는 단계  */
	listen(sockfd,5);//backlog(연결요청대기큐)는 5개까지 대기, server socket 은 연결 대기 상태가 됨 

	clilen = sizeof(cli_addr);//cli_addr 의 크기 clilen에 할당

	char *index_page = 
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:252\r\n\r\n"
		"<!DOCTYPE html>\r\n<html>HELLOWORLD!<br>\r\n"
		"<a href=\"music.mp3\">music</a><br>\r\n"
		"<a href=\"pic.jpg\">image file</a><br>\r\n"
		"<a href=\"doc.pdf\">document</a><br>\r\n"
		"<a href=\"move.gif\">motion picture</a><br></html>\r\n";
	//기본 index 페이지(text\html 타입)에 music, image file, document, motion picture 링크

	while(1){
		if((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))<0)//큐의 연결을 가져와서 새로운 소켓 생성 
		{
			perror("accept");//만약 -1이 반환값일 경우 에러 
			exit(EXIT_FAILURE);
		}


		// if (newsockfd < 0)
		// 	error("ERROR on accept");
		

		bzero(buffer, 512);//buffer 초기화 
	/*#5. read -( read 까지 blocking상태) */
		n = read(newsockfd, buffer ,511);//소켓에서 요청을 읽어와서 buffer 에 넣음

		if (n<0) error("ERROR reading from socket");//실패시 에러처리

		printf("Here is the message: %s\n", buffer);//읽은 요청 프린트 






		/*index.html */
		if (!strncmp(buffer, "GET /index.html",15))//buffer의 처음부터 15글자까지 GET /index.html 일 경우, index 에 해당하는 페이지를 보여줌 
		{	
			write(newsockfd,index_page,strlen(index_page));//newsockfd 에 index_page 를 씀
			write(newsockfd,'\r\n',4);
		}




		/*Music File */	
		else if (!strncmp(buffer,"GET /music.mp3",14))//buffer의 처음부터 14글자까지 "GET /music.mp3" 일 경우, music.mp3에 해당하는 페이지를 보여줌 
		{

	        file = fopen("music.mp3", "rb");//mp3파일을 바이너리 형식으로 열음 
           	fseek(file, 0, SEEK_END);//파일 길이를 가져옴
	        fileLen=ftell(file);
	        fseek(file, 0, SEEK_SET);
           	buf=(char *)malloc(fileLen);//파일 길이에 따라 buf에 공간을 할당함
		
      		fread(buf,fileLen,sizeof(unsigned char),file);//열은 file을 읽어서 buf에 저장 
			 
			char *music_page = "HTTP/1.1 200 OK\r\nContent-Type:audio/mpeg\r\n"; //audio/mpeg 타입으로 헤더 선언
			sprintf(lengthbuf, "Content-Length: %lu\r\n\r\n", fileLen);// lengthbuffer를 Content-length: fileLen으로 설정
			write(newsockfd,music_page, strlen(music_page));//newsockfd 에 music_page 를 씀
			write(newsockfd,lengthbuf,strlen(lengthbuf));//newsockfd에 lengthbuffer(Content-type:--)를 씀
			write(newsockfd,buf,fileLen); //newsockfd 에 buf(파일)을 씀
			fclose(file);//파일을 닫음
			bzero(buf,strlen(buf));//buf를 초기화 

		}
		/*MotionPictiure File */	
		else if (!strncmp(buffer,"GET /move.gif",13))//buffer의 처음부터 13글자까지 "GET /move.gif" 일 경우,move.gif에 해당하는 페이지를 보여줌 
		{

	        file = fopen("move.gif", "rb");//gif파일을 바이너리 형식으로 열음 
           	fseek(file, 0, SEEK_END);//파일 길이를 가져옴
	        fileLen=ftell(file);
	        fseek(file, 0, SEEK_SET);
           	buf=(char *)malloc(fileLen);	//파일 길이에 따라 buf에 공간을 할당함
		
      		fread(buf,fileLen,sizeof(unsigned char),file);//열은 file을 읽어서 buf에 저장 
			 
			char *gif_page = "HTTP/1.1 200 OK\r\nContent-Type:image/gif\r\n";//image/gif 타입으로 헤더 선언
			sprintf(lengthbuf, "Content-Length:%lu\r\n\r\n", fileLen);	// lengthbuffer를 Content-length: fileLen으로 설정
			write(newsockfd,gif_page, strlen(gif_page));//newsockfd 에 gif_page 를 씀
			write(newsockfd,lengthbuf,strlen(lengthbuf));//newsockfd에 lengthbuffer(Content-type:--)를 씀
			write(newsockfd,buf,fileLen);//newsockfd 에 buf(파일)을 씀
			fclose(file);//파일을 닫음
			bzero(buf,strlen(buf));
			//buf를 초기화 


		}
		/*Document(PDF) File */
		else if (!strncmp(buffer,"GET /doc.pdf",12))//buffer의 처음부터 12글자까지 "GET /doc.pdf" 일 경우,doc.pdf에 해당하는 페이지를 보여줌 
		{
				
	        file = fopen("doc.pdf", "rb");//pdf파일을 바이너리 형식으로 열음 
           	fseek(file, 0, SEEK_END);//파일 길이를 가져옴
	        fileLen=ftell(file);
	        fseek(file, 0, SEEK_SET);
	        //Allocate
           	buf=(char *)malloc(fileLen);//파일 길이에 따라 buf에 공간을 할당함	
		
      		fread(buf,fileLen,sizeof(unsigned char),file);//열은 file을 읽어서 buf에 저장 
			 // octet-stream
			char *pdf_page = "HTTP/1.1 200 OK\r\nContent-Type:application/pdf\r\n";//application/pdf 타입으로 헤더 선언
			sprintf(lengthbuf, "Content-Length: %lu\r\n\r\n", fileLen);	// lengthbuffer를 Content-length: fileLen으로 설정
			write(newsockfd,pdf_page, strlen(pdf_page));//newsockfd 에 pdf_page 를 씀
			write(newsockfd,lengthbuf,strlen(lengthbuf));//newsockfd에 lengthbuffer(Content-type:--)를 씀
			write(newsockfd,buf,fileLen);//newsockfd 에 buf(파일)을 씀
			fclose(file);//파일을 닫음
			bzero(buf,strlen(buf));//buf를 초기화 





		}

		/*Image File */
		else if (!strncmp(buffer, "GET /pic.jpg",12))//buffer의 처음부터 13글자까지 "GET /pic.jpg" 일 경우,pic.jpg에 해당하는 페이지를 보여줌 
		{
		    
	        file = fopen("pic.jpg", "rb");
           	fseek(file, 0, SEEK_END);//파일 길이를 가져옴
	        fileLen=ftell(file);
	        fseek(file, 0, SEEK_SET);
	        //Allocate
           	buf=(char *)malloc(fileLen);//파일 길이에 따라 buf에 공간을 할당함	
		
      		fread(buf,fileLen,sizeof(unsigned char),file);//열은 file을 읽어서 buf에 저장 
			 
			char *img_page = "HTTP/1.1 200 OK\r\nContent-Type:image/jpg\r\n";//image/jpg 타입으로 헤더 선언
			sprintf(lengthbuf, "Content-Length: %lu\r\n\r\n", fileLen);	// lengthbuffer를 Content-length: fileLen으로 설정
			write(newsockfd,img_page, strlen(img_page));//newsockfd 에 img_page 를 씀
			write(newsockfd,lengthbuf,strlen(lengthbuf));//newsockfd에 lengthbuffer(Content-type:--)를 씀
			write(newsockfd,buf,fileLen);//newsockfd 에 buf(파일)을 씀
			fclose(file);//파일을 닫음
			bzero(buf,strlen(buf));//buf를 초기화 

		}
		else
			write(newsockfd,index_page,strlen(index_page));//newsockfd에 index_page를 씀



		bzero(buffer,512);//buffer를 초기화 
		write(newsockfd, buffer, 512);//newsockfd에 buffer(빈페이지)를 씀

	}
	close(sockfd);//sockfd닫음
	close(newsockfd);		//newsockfd닫음
	return 0;

}

