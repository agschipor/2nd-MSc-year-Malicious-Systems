import re


def tokenize(data, regex):
    links = re.findall("href=\"([^\"]+)\"", data)
    srcs = re.findall("src=\"(http:[^\"]+)\"", data)

    data = re.sub(r"<[^>]*>", "", data)
    words = regex.findall(data)

    words += links
    words += srcs

    tokens = {}
    for i in range(len(words)):
        tokens[words[i].lower()] = 0

    return tokens


def extract_features(data, keywords):
    regex = re.compile(r"([\w']+)")
    tokens = tokenize(data, regex)
    features = []
    for keyword in keywords:
        if keyword in tokens:
            features.append(1)
        else:
            features.append(0)

    # contains links
    if "http://" in data or "https://" in data:
        features.append(1)
    else:
        features.append(0)

    # de adaugat features
    return features
