import pytest

def pytest_addoption(parser):
    parser.addoption("--logName", action="store", default="x00",
                     help="GDP GCL Log Name")

@pytest.fixture
def logName(request):
    return request.config.getoption("--logName")
