from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="xmss-post-quantum",
    version="1.0.0",
    author="XMSS Development Team",
    author_email="example@example.com",
    description="XMSS-based Post-Quantum Signature Scheme Implementation",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/example/xmss-post-quantum",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Topic :: Security :: Cryptography",
    ],
    python_requires=">=3.8",
    install_requires=[
        "pycryptodome>=3.15.0",
    ],
)