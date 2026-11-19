FROM gcc:14

WORKDIR /app/backend

COPY backend/ ./
COPY frontend/ /app/frontend/

RUN g++ -std=c++17 -pthread main.cpp -o f1server

EXPOSE 10000

CMD ["./f1server"]
