import json
import nltk

from nltk.tokenize import word_tokenize

def handle(req):
    """ Analyzes an english sentence by tokenizing and POS-tagging it. """
    request_json = json.loads(req)
    sentence = request_json["sentence"]

    tokens = word_tokenize(sentence)

    analysis = list()
    for idx, token_and_pos_tag in enumerate(nltk.pos_tag(tokens)):
        token, pos_tag = token_and_pos_tag
        analysis.append({
            "index:": idx,
            "token": token,
            "pos_tag": pos_tag
        })

    return {"sentence": sentence, "tokens": analysis}
