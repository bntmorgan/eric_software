sp 		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

dir	:= $(d)/bios
include	$(dir)/rules.mk
dir	:= $(d)/bios-sstic
include	$(dir)/rules.mk
dir	:= $(d)/libbase
include	$(dir)/rules.mk
dir	:= $(d)/libhal
include	$(dir)/rules.mk
dir	:= $(d)/libnet
include	$(dir)/rules.mk

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
