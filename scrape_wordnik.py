#!/usr/bin/env python

from wordnik import WordApi, swagger
import ConfigParser, argparse, time, csv

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--start_at', default='aaa')
    parser.add_argument('--words_file', default='threeletters.txt')
    parser.add_argument('--outfile', default='defns.txt')
    args = parser.parse_args()

    config = ConfigParser.ConfigParser()
    config.read('config.txt')

    apiUrl = 'http://api.wordnik.com/v4'
    apiKey = config.get('wordnik', 'api_key')
    client = swagger.ApiClient(apiKey, apiUrl)

    words = [l.strip().lower() for l in open(args.words_file)]

    defns = {}
    outfile = open(args.outfile, 'a')
    writer = csv.writer(outfile)

    wordApi = WordApi.WordApi(client)
    for word in words:
        if word < args.start_at:
            continue

        try:
            definitions = wordApi.getDefinitions(word, limit=2)
                                                 # partOfSpeech='verb',
                                                 # sourceDictionaries='wiktionary',
            if definitions:
                writer.writerow((word, definitions[0].text))
            else:
                writer.writerow((word, 'No definition found.'))
            outfile.flush()
        except:
            # TODO this errors on any non-ascii character. Fix that.
            # This is a stopgap just so the script keeps going.
            writer.writerow((word, 'Error finding definition.'))
            outfile.flush()
        time.sleep(5)
