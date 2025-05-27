NAME := apache
CC := g++
CFLAGS := -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS := utils/stringNumber.cpp server/Iconnect.cpp server/server.cpp request/request.cpp \
reponse/Response.cpp \

HEADERS := includes/AResponse.hpp  includes/Response.hpp includes/Iconnect.hpp \
includes/server.hpp includes/utils.hpp includes/webserver.hpp

OBJS := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -g3 -std=c++11 -fsanitize=address $(OBJS) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CC) -c -g3 -std=c++11 -fsanitize=address $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
