FROM alpine
# 安装 ip 命令
RUN apk add --no-cache iproute2
# 其他指令
COPY hello.txt /root/
RUN echo "built inside container" > /built.txt
CMD /bin/sh