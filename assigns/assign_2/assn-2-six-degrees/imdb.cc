using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "imdb.h"
#include <string.h>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

int cmpActors (const void *key, const void *elemAdr) {
  string player = ((tuple_t*)key)->player;
  string to_compare = ((char *)((tuple_t*)key)->start) + *(int*)elemAdr; 
  const char *first = player.c_str();
  const char *second = to_compare.c_str();
  //printf("compares %s with %s; \n", first, second); 
  return strcmp(first, second); 
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const { 
  tuple_t key;
  key.player = player;
  key.start = (void *)actorFile; 
  int *offset = (int *)bsearch(&key, (int*)actorFile + 1, *(int*)actorFile, sizeof(int), cmpActors);
  //printf("offset was %i; \n", *offset);
  if (offset != NULL){
    //printf("Offset is not NULL\n");
    char *actor = (char *)actorFile + *offset;
    int length = (int)strlen(actor) + 1; 
    if ((length % 2) != 0){
      length += 1; 
    } 
    short *nMovies = (short *)(actor + length);
    int *movie_offset_base; 
    if ((length + 2) % 4 == 0){
      movie_offset_base = (int *)((char *)nMovies + 2);
    } else {
      movie_offset_base = (int *)((char *) nMovies + 4);
    }
    //printf("Variables are: \n actor - %s,\n name length is - %i,\n", actor, length);
    //printf("nMovies is - %i,\n movie offset base is - %i\n", (int)*nMovies, *movie_offset_base);

    for (int i = 0; i < *nMovies; i++){
      int *movie_offset = movie_offset_base + i; 
      char *movie = (char *)movieFile + *movie_offset;
      //printf("This movie_offset is - %i,\n movie is - %s\n", *movie_offset, movie);
      film newFilm;
      newFilm.title = movie;
      int movie_title_length = (int)strlen(movie) + 1;
      newFilm.year = 1900 + *(movie + movie_title_length);
      films.push_back(newFilm);
      //printf("His movie is %s, released in - %i\n", newFilm.title.c_str(), newFilm.year);
    }
    return true; 
  }
  //printf("Offset was NULL\n");
  return false; 
}

int cmpMovies (const void *key, const void *movieAdr) {
  film keyMovie = ((tuple_t2*)key)->movie;
  char *movie_to_compare = (char *)(((tuple_t2*)key)->start) + *(int*)movieAdr;
  film m;
  m.title = movie_to_compare; 
  int m_title_len = (int)strlen(movie_to_compare) + 1;
  m.year = 1900 + *(movie_to_compare + m_title_len); 
  if (keyMovie == m) {
    return 0;
  } else if (keyMovie < m) {
    return -1;
  }
  return 1; 
}

bool imdb::getCast(const film& movie, vector<string>& players) const { 
  tuple_t2 key;
  key.movie = movie;
  key.start = (void *)movieFile; 
  int *offset = (int *)bsearch(&key, (int*)movieFile + 1, *(int*)movieFile, sizeof(int), cmpMovies);
  if (offset != NULL){
    //printf("Offset is not NULL\n");
    char *mov = (char *)movieFile + *offset; 
    //printf("Movie name is %s\n", mov); 
    int length = (int)strlen(mov);
    //printf("title length is - %i\n", length);
    if ((length + 2) % 2 != 0) {
      length += 3;
    } else {
      length += 2; 
    }
    //printf("Movie title + 1 byte for the year = %i\n", length);
    short *nActors = (short *)(mov + length);
    //printf("Number of actors in the movie is - %hi\n", *nActors); 
    int *actor_offset_base;
    if ((length + 2) % 4 == 0){
      actor_offset_base = (int *)((char *)nActors + 2); 
    } else {
      actor_offset_base = (int *)((char *)nActors + 4); 
    }
    //printf("Actor_offset_base is %i\n", *actor_offset_base);

    for (int i = 0; i < *nActors; i++){
      int *actor_offset = actor_offset_base + i; 
      char *actor = (char *)actorFile + *actor_offset;
      players.push_back(actor); 
      //printf("The actor is %s\n", actor);
    }
    return true; 
  }
  //printf("Offset is NULL");
  return false; 
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}