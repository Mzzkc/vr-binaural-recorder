# Dockerfile - Multi-stage build for VR Binaural Recorder
# Build stage
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    ninja-build \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libasound2-dev \
    libjack-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Clone and build PortAudio
RUN git clone https://github.com/PortAudio/portaudio.git && \
    cd portaudio && \
    cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DPA_BUILD_SHARED=OFF && \
    cmake --build build && \
    cmake --install build

# Clone and build spdlog
RUN git clone https://github.com/gabime/spdlog.git && \
    cd spdlog && \
    cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DSPDLOG_BUILD_SHARED=OFF && \
    cmake --build build && \
    cmake --install build

# Clone and build jsoncpp
RUN git clone https://github.com/open-source-parsers/jsoncpp.git && \
    cd jsoncpp && \
    cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF && \
    cmake --build build && \
    cmake --install build

# Download OpenVR SDK
RUN wget https://github.com/ValveSoftware/openvr/archive/refs/tags/v1.23.8.tar.gz && \
    tar -xzf v1.23.8.tar.gz && \
    mv openvr-1.23.8 /opt/openvr

# Clone and build ImGui
RUN git clone https://github.com/ocornut/imgui.git /opt/imgui

# Copy application source
COPY . /build/vr_binaural_recorder

# Build application
WORKDIR /build/vr_binaural_recorder
RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DOPENVR_ROOT_DIR=/opt/openvr \
    -DIMGUI_PATH=/opt/imgui && \
    cmake --build build

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libgl1 \
    libglu1-mesa \
    libx11-6 \
    libxrandr2 \
    libxi6 \
    libasound2 \
    libjack0 \
    && rm -rf /var/lib/apt/lists/*

# Create application directory
WORKDIR /app

# Copy built application and resources
COPY --from=builder /build/vr_binaural_recorder/build/vr_binaural_recorder /app/
COPY --from=builder /opt/openvr/bin/linux64/libopenvr_api.so /app/
COPY --from=builder /build/vr_binaural_recorder/vr_binaural_config.json.example /app/vr_binaural_config.json
COPY --from=builder /build/vr_binaural_recorder/hrtf_data /app/hrtf_data
COPY --from=builder /build/vr_binaural_recorder/README.md /app/

# Create required directories
RUN mkdir -p /app/logs

# Set library path
ENV LD_LIBRARY_PATH=/app:$LD_LIBRARY_PATH

# Entry point
ENTRYPOINT ["/app/vr_binaural_recorder"]

# ================================
# docker-compose.yml - Container orchestration
# ================================

---
version: '3.8'

services:
  vr-binaural-recorder:
    build: .
    image: vrb/binaural-recorder:latest
    container_name: vr_binaural_recorder
    
    # Required for audio and GPU access
    privileged: true
    
    # Mount points
    volumes:
      - ./config:/app/config:ro
      - ./logs:/app/logs:rw
      - ./hrtf_data:/app/hrtf_data:ro
      - /dev/snd:/dev/snd:rw
      - /tmp/.X11-unix:/tmp/.X11-unix:rw
      
    # Environment variables
    environment:
      - DISPLAY=${DISPLAY}
      - PULSE_SERVER=unix:/tmp/pulse-socket
      - VR_CONFIG_PATH=/app/config/vr_binaural_config.json
      
    # Device access
    devices:
      - /dev/dri:/dev/dri
      - /dev/snd:/dev/snd
      
    # Network mode for VR communication
    network_mode: host
    
    # Resource limits
    deploy:
      resources:
        limits:
          cpus: '2.0'
          memory: 1G
        reservations:
          cpus: '1.0'
          memory: 512M
          
    # Restart policy
    restart: unless-stopped
    
    # Logging
    logging:
      driver: json-file
      options:
        max-size: "10m"
        max-file: "3"

  # Optional: Virtual audio cable service
  virtual-audio:
    image: vrb/virtual-audio:latest
    container_name: virtual_audio_cable
    privileged: true
    
    volumes:
      - /dev/snd:/dev/snd:rw
      
    environment:
      - CABLE_NAME=VR Binaural Output
      - SAMPLE_RATE=48000
      - CHANNELS=2
      
    devices:
      - /dev/snd:/dev/snd
      
    restart: unless-stopped

# ================================
# kubernetes.yaml - Kubernetes deployment
# ================================

---
apiVersion: v1
kind: Namespace
metadata:
  name: vr-audio
  
---
apiVersion: v1
kind: ConfigMap
metadata:
  name: vr-binaural-config
  namespace: vr-audio
data:
  vr_binaural_config.json: |
    {
      "audio": {
        "sampleRate": 48000,
        "bufferSize": 128,
        "preferredInputDevice": "",
        "virtualOutputName": "VR Binaural Output"
      },
      "hrtf": {
        "dataPath": "/app/hrtf_data"
      },
      "logging": {
        "level": "info",
        "path": "/app/logs"
      },
      "vr": {
        "overlayScale": 0.5
      }
    }
    
---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: vr-binaural-logs
  namespace: vr-audio
spec:
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 10Gi
      
---
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: vr-binaural-recorder
  namespace: vr-audio
  labels:
    app: vr-binaural-recorder
spec:
  selector:
    matchLabels:
      app: vr-binaural-recorder
  template:
    metadata:
      labels:
        app: vr-binaural-recorder
    spec:
      hostNetwork: true
      hostPID: true
      
      containers:
      - name: vr-binaural-recorder
        image: vrb/binaural-recorder:latest
        imagePullPolicy: Always
        
        securityContext:
          privileged: true
          
        env:
        - name: DISPLAY
          value: ":0"
        - name: VR_CONFIG_PATH
          value: "/config/vr_binaural_config.json"
          
        volumeMounts:
        - name: config
          mountPath: /config
        - name: logs
          mountPath: /app/logs
        - name: x11-unix
          mountPath: /tmp/.X11-unix
        - name: sound
          mountPath: /dev/snd
        - name: dri
          mountPath: /dev/dri
          
        resources:
          limits:
            memory: "1Gi"
            cpu: "2000m"
            nvidia.com/gpu: "1"  # For NVIDIA GPU
          requests:
            memory: "512Mi"
            cpu: "1000m"
            
      volumes:
      - name: config
        configMap:
          name: vr-binaural-config
      - name: logs
        persistentVolumeClaim:
          claimName: vr-binaural-logs
      - name: x11-unix
        hostPath:
          path: /tmp/.X11-unix
      - name: sound
        hostPath:
          path: /dev/snd
      - name: dri
        hostPath:
          path: /dev/dri
          
      nodeSelector:
        vr-enabled: "true"
        
---
apiVersion: v1
kind: Service
metadata:
  name: vr-binaural-metrics
  namespace: vr-audio
spec:
  selector:
    app: vr-binaural-recorder
  ports:
    - name: metrics
      port: 9090
      targetPort: 9090
      protocol: TCP
  type: ClusterIP

# ================================
# deployment.sh - Deployment script
# ================================

#!/bin/bash
set -e

# Configuration
REGISTRY="docker.io/vrbaudio"
IMAGE_NAME="vr-binaural-recorder"
VERSION="${1:-latest}"

echo "Building VR Binaural Recorder v${VERSION}..."

# Build Docker image
docker build -t ${REGISTRY}/${IMAGE_NAME}:${VERSION} .
docker tag ${REGISTRY}/${IMAGE_NAME}:${VERSION} ${REGISTRY}/${IMAGE_NAME}:latest

echo "Running tests..."
docker run --rm ${REGISTRY}/${IMAGE_NAME}:${VERSION} --test

echo "Pushing to registry..."
docker push ${REGISTRY}/${IMAGE_NAME}:${VERSION}
docker push ${REGISTRY}/${IMAGE_NAME}:latest

# Deploy to Kubernetes if kubectl is available
if command -v kubectl &> /dev/null; then
    echo "Deploying to Kubernetes..."
    kubectl apply -f kubernetes.yaml
    kubectl rollout status daemonset/vr-binaural-recorder -n vr-audio
    echo "Deployment complete!"
else
    echo "kubectl not found, skipping Kubernetes deployment"
fi

# Deploy with Docker Compose for local testing
if command -v docker-compose &> /dev/null; then
    echo "Starting local instance with Docker Compose..."
    docker-compose up -d
    echo "Local instance running!"
fi

echo "VR Binaural Recorder ${VERSION} deployed successfully!"

# ================================
# .github/workflows/build.yml - CI/CD Pipeline
# ================================

name: Build and Deploy

on:
  push:
    branches: [ main, develop ]
    tags: [ 'v*' ]
  pull_request:
    branches: [ main ]

env:
  REGISTRY: ghcr.io
  IMAGE_NAME: ${{ github.repository }}

jobs:
  build:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      packages: write
      
    steps:
    - name: Checkout repository
      uses: actions/checkout@v3
      with:
        submodules: recursive
        
    - name: Set up Docker Buildx
      uses: docker/setup-buildx-action@v2
      
    - name: Log into registry ${{ env.REGISTRY }}
      if: github.event_name != 'pull_request'
      uses: docker/login-action@v2
      with:
        registry: ${{ env.REGISTRY }}
        username: ${{ github.actor }}
        password: ${{ secrets.GITHUB_TOKEN }}
        
    - name: Extract metadata
      id: meta
      uses: docker/metadata-action@v4
      with:
        images: ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}
        tags: |
          type=ref,event=branch
          type=ref,event=pr
          type=semver,pattern={{version}}
          type=semver,pattern={{major}}.{{minor}}
          type=sha
          
    - name: Build and push Docker image
      uses: docker/build-push-action@v4
      with:
        context: .
        platforms: linux/amd64
        push: ${{ github.event_name != 'pull_request' }}
        tags: ${{ steps.meta.outputs.tags }}
        labels: ${{ steps.meta.outputs.labels }}
        cache-from: type=gha
        cache-to: type=gha,mode=max
        
    - name: Run tests in container
      run: |
        docker run --rm ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}:${{ steps.meta.outputs.version }} --test
        
  deploy:
    needs: build
    runs-on: ubuntu-latest
    if: github.event_name == 'push' && contains(github.ref, 'refs/tags/')
    
    steps:
    - name: Deploy to production
      env:
        KUBE_CONFIG: ${{ secrets.KUBE_CONFIG }}
      run: |
        echo "$KUBE_CONFIG" | base64 -d > kubeconfig
        export KUBECONFIG=kubeconfig
        kubectl rollout restart daemonset/vr-binaural-recorder -n vr-audio
        kubectl rollout status daemonset/vr-binaural-recorder -n vr-audio