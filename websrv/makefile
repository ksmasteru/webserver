NAME := apache
CC := c++
CFLAGS := -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS := handle_https_request/delete.cpp handle_https_request/get.cpp handle_https_request/post.cpp \
	server/establish_server.cpp utils/stringNumber.cpp

HEADERS := webserver.hpp

OBJS := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all