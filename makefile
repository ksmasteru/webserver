NAME := apache
CC := g++
CFLAGS := -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS := utils/stringNumber.cpp server/Iconnect.cpp server/server.cpp request/request.cpp \
reponse/Response.cpp server/serverManager.cpp utils/trim.cpp reponse/cgiHandler.cpp \

HEADERS := includes/webserver.hpp includes/AResponse.hpp includes/Iconnect.hpp includes/Response.hpp \
includes/server.hpp includes/utils.hpp includes/webserver.hpp includes/Location.hpp \
includes/serverManager.hpp includes/ConfigParser.hpp includes/Location.hpp includes/Connection.hpp \
includes/cgiHandler.hpp
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
