package main

import (
	"fmt"
	"io"
	"log"
	"net/http"
	"net/url"
	"os"
	"path"
	"strings"

	"golang.org/x/net/html"
	"golang.org/x/net/html/atom"
)

func save(link string, content []byte) {
	u, err := url.Parse(link)
	if err != nil {
		log.Fatal(err)
	}
	link = u.Host + u.Path
	os.Mkdir(link, 0777)

	if u.Path == "" {
		link += "/"
	}

	if strings.HasSuffix(link, "/") {
		link += "index.html"
	}

	os.WriteFile(link, content, 0777)

	fmt.Println("=========> save", link)

}

var visited = map[string]bool{}

func crawl(link string) {
	u, err := url.Parse(link)
	if err != nil {
		log.Fatal(err)
	}

	trailing := strings.HasSuffix(u.Path, "/")
	u.Path = path.Clean("/" + u.Path)
	if trailing && u.Path != "/" {
		u.Path += "/"
	}

	key := u.Host + u.Path
	if visited[key] {
		return
	}
	visited[key] = true

	client := &http.Client{}

	req, err := http.NewRequest("GET", u.String(), nil)
	if err != nil {
		log.Fatal(err)
	}

	req.Header.Add("User-Agent", `Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36`)
	resp, err := client.Do(req)
	if err != nil {
		log.Fatal(err)
	}

	defer resp.Body.Close()
	bodyByte, err := io.ReadAll(resp.Body)

	body := string(bodyByte)
	save(link, bodyByte)

	doc, err := html.Parse(strings.NewReader(body))
	if err != nil {
		log.Fatal(err)
	}
	for n := range doc.Descendants() {
		if n.Type == html.ElementNode && n.DataAtom == atom.A {
			for _, a := range n.Attr {
				if a.Key == "href" {
					ref, err := url.Parse(a.Val)
					if err == nil && !ref.IsAbs() && !strings.HasPrefix(a.Val, "#") {
						crawl(u.ResolveReference(ref).String())
					}
					break
				}
			}
		}
	}

}

func main() {
	if len(os.Args) < 2 {
		log.Fatal("usage: wgo <link>")
	}
	link := os.Args[1]
	crawl(link)
}
