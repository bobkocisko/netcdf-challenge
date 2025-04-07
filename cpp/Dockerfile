# Thanks to 
# https://stackoverflow.com/questions/69011431/building-and-deploying-c-through-docker-multistage-build-vs-mount
# for providing the template for this


# ---------
# Stage #1
# ---------

# We start with a base os stage that is updated and that
# handles pid 1 issues so we don't have to do that multiple times
FROM ubuntu:24.04 AS os_base
RUN apt-get update

# Install any base dependencies we need here.
RUN DEBIAN_FRONTEND=noninteractive \
  apt-get install --assume-yes --no-install-recommends \
  tini

# Make sure that PID 1 is a real init process
# so that signals are properly forwarded and zombies
# are taken care of
ENTRYPOINT ["/usr/bin/tini", "--"]


# ---------
# Stage #2
# ---------

# The dev environment stage is used both as a starting point 
# for the builder stage below and also as a dev container for the ide
FROM os_base AS dev_environment

# Install any base dependencies we need here for building both
# the dependent libraries and our own library.
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get install --assume-yes --no-install-recommends \
      build-essential \
      cmake \
      pkg-config \
      libnetcdf-c++4-dev


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

# Build matplot++ library and install it into /usr/local.
WORKDIR /usr/src/matplotplusplus
COPY third_party/matplotplusplus ./
WORKDIR /usr/src/matplotplusplus/build
RUN cmake .. \
      -DMATPLOTPP_BUILD_EXAMPLES=OFF      \
      -DMATPLOTPP_BUILD_SHARED_LIBS=ON    \
      -DMATPLOTPP_BUILD_TESTS=OFF         \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
RUN make install

# NOTE: at the end of this stage we now have an environment that
# can be used by the IDE with all dependencies fully present,
# and can also be used by the next stage to build the final 
# executable




# ---------
# Stage #3
# ---------


# The builder stage continues from the dev_environment stage and
# builds the application
FROM dev_environment AS builder

# Build the application itself
WORKDIR /usr/src/app
COPY src /usr/src/app/
WORKDIR /usr/src/app/build
RUN cmake ..

RUN make install


# ---------
# Stage #4
# ---------

# The final FROM line is the actual runtime image.  Anything before
# this will be built, and will show up in `docker images`, but isn't
# "the output" of the build.

FROM os_base AS final_image

ARG NETCDF_FILE
ARG NETCDF_FILE_TARGET

# Install runtime libaries for dependencies
RUN DEBIAN_FRONTEND=noninteractive \
  apt-get install --assume-yes --no-install-recommends \
  libnetcdf-c++4-1 \
  gnuplot


# Copy the installed libraries from the earlier stage.
COPY --from=builder /usr/local/ /usr/local/

# Regenerate the shared-library cache.
RUN ldconfig

# Add the netCDF file to run against
COPY ${NETCDF_FILE} ${NETCDF_FILE_TARGET}

# Set ordinary metadata to run a Docker container.  (The binary is
# probably in /usr/local/bin which is on the default PATH.)
EXPOSE 8080

# NOTE: the expectation is that this command will be run by
#  docker-compose.yml specifying a 'command'.  Unfortunately 
#  I wasn't able to find a way to have the following line 
#  resolve to the value of ARG (or an ENV) so I ultimately gave 
#  up on this in favor of duplicative configuration in 
#  docker-compose.yml
# CMD ["netcdf_api", "/usr/local/var/${NETCDF_FILE}"]
