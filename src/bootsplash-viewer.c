// https://gitlab.manjaro.org/manjaro-arm/packages/extra/bootsplash-viewer

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "bootsplash_file.h"
typedef struct splash_file_header FileHeader;
typedef struct splash_pic_header PicHeader;
typedef struct splash_blob_header BlobHeader;

// TODO make this command line args?
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

typedef struct Image {
	SDL_Rect geometry;
	int xoff, yoff;
	bool center_x, center_y;
	size_t num_frames;
	uint32_t* frames;
	bool animated;
	uint32_t anim_start;
	uint32_t cur_frame;
	SDL_Texture* texture;
} Image;

Image* images;

FileHeader header;
SDL_Window* window;
SDL_Renderer* renderer;


/**
 * Make the image position easier to work with
 *
 * @param pos The original position
 * @return bit 0 is set if from bottom, bit 1 if from right
 */
uint8_t fix_position(enum splash_position pos) {
	pos = (pos & 0xF);
	uint8_t r = 0;
	if (pos == SPLASH_CORNER_BOTTOM || pos == SPLASH_CORNER_BOTTOM_LEFT || pos == SPLASH_CORNER_BOTTOM_RIGHT)
		r |= 0b10;
	if (pos == SPLASH_CORNER_RIGHT || pos == SPLASH_CORNER_TOP_RIGHT || pos == SPLASH_CORNER_BOTTOM_RIGHT)
		r |= 0b01;
	if (pos == SPLASH_CORNER_TOP || pos == SPLASH_CORNER_BOTTOM)
		r |= 0b0100;
	if (pos == SPLASH_CORNER_LEFT || pos == SPLASH_CORNER_RIGHT)
		r |= 0b1000;

	return r;
}

void render() {
	SDL_RenderClear(renderer);
	int ww, wh;
	SDL_GetWindowSize(window, &ww, &wh);

	for (int i = 0; i < header.num_pics; i++) {
		Image* image = &images[i];
		uint8_t* pixels;
		int pitch;

		SDL_LockTexture(image->texture, NULL, (void*) &pixels, &pitch);
		memcpy(pixels, &image->frames[image->cur_frame * image->geometry.w * image->geometry.h], image->geometry.w * image->geometry.h * 4);
		SDL_UnlockTexture(image->texture);

		image->cur_frame++;
		if (image->cur_frame == image->num_frames)
			image->cur_frame = image->anim_start;

		SDL_Rect pos;
		pos.w = image->geometry.w;
		pos.h = image->geometry.h;

		if (image->center_x)
			pos.x = ((ww / 2) - (pos.w / 2)) + image->xoff;
		else if (image->xoff >= 0)
			pos.x = image->xoff;
		else
			pos.x = ww - pos.w + (image->xoff + 1);

		if (image->center_y)
			pos.y = ((wh / 2) - (pos.h / 2)) + image->yoff;
		else if (image->yoff >= 0)
			pos.y = image->yoff;
		else
			pos.y = wh - pos.h + (image->yoff + 1);

		SDL_RenderCopy(renderer, image->texture, NULL, &pos);
	}

	SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <theme-file>\n", argv[0]);
		exit(1);
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		fprintf(stderr, "Could not init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	FILE* splash_file = fopen(argv[1], "r");
	if (!splash_file) {
		perror(argv[1]);
	}

	size_t read = fread(&header, sizeof(FileHeader), 1, splash_file);

	if (read < 1) {
		fprintf(stderr, "Could not read header.\n");
		exit(1);
	}

	if (memcmp(header.id, BOOTSPLASH_MAGIC_LE, 16) != 0) {
		fprintf(stderr, "Invalid header: %.16s\n", header.id);
		exit(1);
	}

	window = SDL_CreateWindow(
		"Bootsplash Viewer", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP
	);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(renderer, header.bg_red, header.bg_green, header.bg_blue, 0);

	images = calloc(header.num_pics, sizeof(Image));
	for (int i = 0; i < header.num_pics; i++) {
		PicHeader pic;

		size_t pread = fread(&pic, sizeof(PicHeader), 1, splash_file);
		if (pread > 1) {
			fprintf(stderr, "Could not read pic header %d.\n", i);
		}

		Image* image = &images[i];
		image->num_frames = pic.num_blobs;
		image->anim_start = pic.anim_loop;
		image->animated = pic.anim_type == SPLASH_ANIM_LOOP_FORWARD;
		image->geometry.w = pic.width;
		image->geometry.h = pic.height;

		bool center = (~pic.position & 0x10) >> 4;
		uint8_t pos = fix_position(pic.position);

		image->center_x = ((pos & 0b0100) >> 2) | center;
		image->center_y = ((pos & 0b1000) >> 3) | center;
		image->xoff = pos & 0b0100 ? 0 : pic.position_offset;
		image->yoff = pos & 0b1000 ? 0 : pic.position_offset;

		// Need to swap the offsets the other way around if centered due to how the format works
		if (center) {
			if (~pos & 0b01) image->xoff = -(image->xoff);
			if (~pos & 0b10) image->yoff = -(image->yoff);
		} else {
			if (pos & 0b01) image->xoff = -(image->xoff + 1);
			if (pos & 0b10) image->yoff = -(image->yoff + 1);
		}
		// Image frames are stored as 24-bit RGB.
		image->frames = calloc(image->num_frames, image->geometry.w * image->geometry.h * 4);
		image->texture = SDL_CreateTexture(
			renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
			image->geometry.w, image->geometry.h
		);
	}

	for (int i = 0; i < header.num_blobs; i++) {
		BlobHeader blob;
		int blobread = fread(&blob, sizeof(BlobHeader), 1, splash_file);
		if (blobread < 1) {
			fprintf(stderr, "Could not read blob %d\n", i);
			exit(1);
		}

		if (blob.picture_id > header.num_pics) {
			fprintf(stderr, "image ID %d out of bounds, have %d images", blob.picture_id, header.num_pics);
			exit(1);
		}

		Image* image = &images[blob.picture_id];
		uint8_t tmp[blob.length];

		int imgread = fread(&tmp, 1, blob.length, splash_file);

		if (imgread < blob.length) {
			fprintf(stderr, "Could not read blob %d\n", i);
			exit(1);
		}

		SDL_ConvertPixels(
			image->geometry.w, image->geometry.h,
			SDL_PIXELFORMAT_RGB24, &tmp, image->geometry.w * 3,
			SDL_PIXELFORMAT_RGBA32, &image->frames[(image->cur_frame++) * (image->geometry.w * image->geometry.h)], image->geometry.w * 4
		);

		// blob size must be a multiple of 16
		if (imgread % 16)
			fseek(splash_file, 16 - (imgread % 16), SEEK_CUR);
	}

	for (int i = 0; i < header.num_pics; i++) {
		images[i].cur_frame = 0;
	}

	SDL_ShowWindow(window);

	bool running = true;
	while (running) {
		SDL_Event e;

		render();

		while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN ) {
				running = false;
	 		} else if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_f) {
					int fs = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
					SDL_SetWindowFullscreen(window, fs ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
				}
			}
		}

		usleep(header.frame_ms * 1000);
	}

	return 0;
}
