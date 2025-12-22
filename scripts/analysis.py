#!/usr/bin/env python3

import pandas as pd
import sqlite3

from pathlib import Path

data_dir = Path("./data/analysis")
data_dir.mkdir(parents=True, exist_ok=True)

df_path = data_dir / "messages.pkl"

if not df_path.exists():
    conn = sqlite3.connect("data/msg.sqlite")

    query = "SELECT createdAt,content,authorId FROM messages"
    df = pd.read_sql_query(query, conn)

    conn.close()

    df["createdAt"] = pd.to_datetime(df["createdAt"])

    df.to_pickle(df_path)
else:
    df = pd.read_pickle(df_path)

df["content"] = df["content"].astype(str)

MAX_CHAR_LENGTH = 600
MAX_NEWLINES = 15
MIN_UNIQUE_RATIO = 0.4


def is_spam(text):
    if len(text) > MAX_CHAR_LENGTH:
        return True
    if text.count("\n") > MAX_NEWLINES:
        return True

    words = text.split()

    if len(words) < 5:
        return False

    unique_words = set(words)
    ratio = len(unique_words) / len(words)

    if ratio < MIN_UNIQUE_RATIO:
        return True

    return False


df_clean = df[~df["content"].apply(is_spam)].copy()
print(f"Original Count: {len(df)}")
print(f"Cleaned Count:  {len(df_clean)}")

removed_rows = df[df["content"].apply(is_spam)]
print("\n--- Examples of REMOVED messages ---")
print(removed_rows.head(10))

print("\n--- Samples of CLEANED messages ---")
print(
    df_clean.sort_values(
        by="content", key=lambda x: x.str.len(), ascending=False
    ).head()
)
