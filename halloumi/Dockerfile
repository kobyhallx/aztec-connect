FROM 278380418400.dkr.ecr.eu-west-2.amazonaws.com/barretenberg-x86_64-linux-clang
FROM 278380418400.dkr.ecr.eu-west-2.amazonaws.com/barretenberg.js

FROM node:16-alpine3.15 AS builder
RUN apk update && apk add --no-cache build-base python3 llvm10-libs bash git
COPY --from=0 /usr/src/barretenberg/build/bin/rollup_cli /usr/src/barretenberg/build/bin/rollup_cli
COPY --from=1 /usr/src/barretenberg.js/dest /usr/src/barretenberg.js/dest
WORKDIR /usr/src/halloumi
COPY . .
RUN yarn install --frozen-lockfile && \
  yarn test && \
  yarn build && \
  rm -rf node_modules && \
  yarn install --frozen-lockfile --production && \
  yarn cache clean

FROM node:16-alpine3.15
RUN apk update && apk add --no-cache llvm10-libs
COPY --from=builder /usr/src /usr/src
WORKDIR /usr/src/halloumi
CMD ["yarn", "start"]
EXPOSE 80