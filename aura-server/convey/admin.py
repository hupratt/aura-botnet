from django.contrib import admin

from .models import Bot, Command, File, Bot_Command

class FileInline(admin.StackedInline):
    model = File
    extra = 1
# class Bot_Command(admin.ModelAdmin):
#     inlines = [
#         FileInline,
#     ]


class CommandAdmin(admin.ModelAdmin):
    inlines = [
        FileInline,
    ]

# Register your models here.
admin.site.register(Bot)
admin.site.register(Command, CommandAdmin)
# admin.site.register(Bot_Command)
