import requests
import json
from datetime import datetime

# ============================================
# CONFIGURATION - Change these as needed
# ============================================
BASE_URL = "http://localhost:27750"
API_KEY = ""  # Add your API key if needed
TIMEOUT = 5  # Request timeout in seconds

# Headers (common for all requests)
HEADERS = {
    "Content-Type": "application/json",
    "Accept": "application/json",
    # "Authorization": f"Bearer {API_KEY}",  # Uncomment if needed
}

TESTING_MODULE = "testing"

# Variables you can use in requests
VARS = {
    "entityName": "TestEntity0",
    "componentName": f"{TESTING_MODULE}.UnitTest",
    "systemName": "testSystem",
    "timesToRun": 1
}

# ============================================
# REQUEST DEFINITIONS
# ============================================
requests_to_run = [
    {
        "name": "Create entity",
        "method": "PUT",
        "url": f"{BASE_URL}/entity/{VARS['entityName']}",
        "headers": HEADERS,
    },
    {
        "name": "Create component",
        "method": "PUT",
        "url": f"{BASE_URL}/component/{VARS['entityName']}",
        # "url": f"{BASE_URL}/component/{VARS['entityName']}?component=testing.UnitTest&value={{%22systemName%22:%22testSystem%22}}",
        "headers": HEADERS,
        "params": {
            "component": VARS["componentName"],
            "value": f"{{\"systemName\":\"{VARS['systemName']}\"}}",
            #"value": "{\"systemName\":\"testSystem\"}",
            # "value": str({"systemName": VARS["systemName"]}),
            #"value": '{"systemName":{}}'.format(VARS["systemName"]),
            #"value": "{\"systemName\":{}}".format("testSystem"),
        },
    },
]

# http://localhost:27750/component/TestEntity0?component=testing.UnitTest&value={%22systemName%22:%22testSystem%22}

# http://localhost:27750/component/TestEntity0?component=testing.UnitTest&value=%7B%27systemName%27%3A+testSystem%7D
# http://localhost:27750/component/TestEntity0?component=testing.UnitTest&value=%7B%22systemName%22:%22testSystem%22%7D // GOOD

#                                                                         value=%7B%22systemName%22%3AtestSystem%7D

# http://localhost:27750/component/TestEntity0?component=testing.UnitTest&value=%7B%27systemName%27%3A+%27testSystem%27%7D


# ============================================
# EXECUTION
# ============================================
def print_separator():
    print("\n" + "=" * 80 + "\n")


def run_requests():
    print(
        f"\n🚀 Starting HTTP Request Runner - {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}"
    )
    print_separator()

    results = []

    for i, req in enumerate(requests_to_run, 1):
        print(f"[{i}/{len(requests_to_run)}] {req['name']}")
        print(f"Method: {req['method']}")
        # print(f"URL: {req['url']}")

        try:
            # Make the request
            prepared = requests.Request(
                method=req["method"],
                url=req["url"],
                headers=req.get("headers", {}),
                params=req.get("params", {}),
                json=req.get("json"),
            ).prepare()

            # Print the final URL with encoded params
            print(f"URL: {prepared.url}")

            # Make the request
            response = requests.Session().send(prepared, timeout=TIMEOUT)

            # Print status
            status_emoji = "✅" if response.ok else "❌"
            print(f"Status: {status_emoji} {response.status_code}")

            # Print response
            print("\nResponse:")
            if response.text:
                try:
                    print(json.dumps(response.json(), indent=2))
                except json.JSONDecodeError:
                    print(response.text)
            else:
                print("(empty response)")

            # Store result
            results.append(
                {
                    "name": req["name"],
                    "status": response.status_code,
                    "success": response.ok,
                    "response": response.text,
                }
            )

        except requests.exceptions.Timeout:
            print(f"Status: ❌ TIMEOUT (>{TIMEOUT}s)")
            results.append({"name": req["name"], "status": "timeout", "success": False})

        except requests.exceptions.RequestException as e:
            print(f"Status: ❌ ERROR")
            print(f"Error: {str(e)}")
            results.append(
                {
                    "name": req["name"],
                    "status": "error",
                    "success": False,
                    "error": str(e),
                }
            )

        print_separator()

    # Summary
    print("📊 SUMMARY")
    print("-" * 80)
    successful = sum(1 for r in results if r.get("success", False))
    print(f"Total Requests: {len(results)}")
    print(f"Successful: {successful}")
    print(f"Failed: {len(results) - successful}")
    print_separator()


if __name__ == "__main__":
    run_requests()
