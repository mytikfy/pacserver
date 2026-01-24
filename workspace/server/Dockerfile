FROM archlinux:base

COPY dist/Debug/GNU-Linux/pacproxy /usr/bin/pacproxy

RUN pacman-key --init
RUN pacman -Sy
RUN pacman -S archlinux-keyring --noconfirm
RUN pacman -Suy --noconfirm
RUN rm -rf /var/cache/pacman/pkg/*

# ENTRYPOINT ["/usr/bin/pacproxy --port 8280"]
