ARG NODE_IMAGE=node:22-bookworm
FROM ${NODE_IMAGE}
ARG INFRA_REF=4fb38b9bbddecf2b7b3533fd740981893e3c532f
WORKDIR /opt/pixel-streaming
RUN git init . && git remote add origin https://github.com/EpicGames/PixelStreamingInfrastructure.git \
    && git fetch --depth 1 origin ${INFRA_REF} && git checkout --detach FETCH_HEAD
RUN npm ci --no-audit --no-fund \
    && npm run build:cjs --workspace Common \
    && npm run build:cjs --workspace Signalling \
    && npm run build --workspace SignallingWebServer \
    && npm run build:esm --workspace Common \
    && npm run build:esm --workspace Frontend/library \
    && npm run build:esm --workspace Frontend/ui-library \
    && npm run build:prod --workspace Frontend/implementations/typescript
COPY configure-frontend.cjs /tmp/configure-frontend.cjs
RUN node /tmp/configure-frontend.cjs && npm run build:prod --workspace Frontend/implementations/typescript
WORKDIR /opt/pixel-streaming/SignallingWebServer
EXPOSE 8080 8888
ENTRYPOINT ["node", "dist/index.js", "--config_file", "/config/signalling.json"]
