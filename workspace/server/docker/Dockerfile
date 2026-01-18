FROM archlinux:base

COPY dist/Debug/GNU-Linux/server /bin/server

RUN pacman-key --init
RUN pacman -Sy
RUN pacman -S archlinux-keyring --noconfirm
RUN pacman -Suy --noconfirm
RUN rm -rf /var/cache/pacman/pkg/*

# RUN apk add libgcc
# RUN apk add libstdc++
# RUN apk add gcc

# ENTRYPOINT ["/bin/server --"]
