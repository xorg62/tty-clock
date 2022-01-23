FROM docker.io/alpine:3.15
RUN apk add -U build-base ncurses-dev
WORKDIR /src
ADD . /src
RUN make

FROM alpine
RUN apk add -U ncurses-libs
COPY --from=0 /src/tty-clock /usr/bin/tty-clock
CMD ['/usr/bin/tty-clock']
