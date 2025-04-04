# Thanks to 
# https://stackoverflow.com/questions/69011431/building-and-deploying-c-through-docker-multistage-build-vs-mount
# for providing the template for this



# The first stage only builds things.  We'll COPY files out of it
# later.
FROM ubuntu:24.04 AS builder


# Install any base dependencies we need here.
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get install --assume-yes --no-install-recommends \
      build-essential \
      cmake



# Build crow library and install it into /usr/local.
RUN DEBIAN_FRONTEND=noninteractive \
  apt-get install --assume-yes --no-install-recommends \
  libasio-dev

WORKDIR /usr/src/libcrow
COPY third_party/libcrow ./
WORKDIR /usr/src/libcrow/build
RUN cmake .. \
      -DCROW_BUILD_EXAMPLES=OFF \
      -DCROW_BUILD_TESTS=OFF

RUN make install



# Build the application itself
RUN DEBIAN_FRONTEND=noninteractive \
  apt-get install --assume-yes --no-install-recommends \
  libnetcdf-c++4-dev

WORKDIR /usr/src/app
COPY src /usr/src/app/
WORKDIR /usr/src/app/build
RUN cmake ..

RUN make install



# The final FROM line is the actual runtime image.  Anything before
# this will be built, and will show up in `docker images`, but isn't
# "the output" of the build.

FROM ubuntu:24.04

# Get the installed libraries from the earlier stage.
COPY --from=builder /usr/local/ /usr/local/

# Regenerate the shared-library cache.
RUN ldconfig

# Set ordinary metadata to run a Docker container.  (The binary is
# probably in /usr/local/bin which is on the default PATH.)
EXPOSE 80
CMD ["netcdf_api"]
