#!/usr/bin/env python3

import os
import sys
import dotenv

if __name__ == "__main__":
    os.environ.setdefault("DJANGO_SETTINGS_MODULE", "aura.settings")
    dotenv.read_dotenv(os.path.join(os.path.dirname(os.path.abspath(__file__)),'.env'))

    from django.core.management import execute_from_command_line

    execute_from_command_line(sys.argv)
