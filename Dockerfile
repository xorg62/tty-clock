FROM docker.io/alpine:3.15
RUN apk add -U build-base ncurses-dev
WORKDIR /src
ADD . /src
RUN make

FROM docker.io/alpine:3.15
ARG TIMEZONE=UTC
COPY --from=0 /src/tty-clock /usr/bin/tty-clock
CMD ['/usr/bin/tty-clock']
RUN apk add -U ncurses-libs tzdata && \
    cp -rf /usr/share/zoneinfo/${TIMEZONE} /etc/localtime

